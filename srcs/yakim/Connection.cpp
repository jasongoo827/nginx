#include "Connection.hpp"
#include "Utils.hpp"
#include "Server.hpp"
#include "Parser.hpp"
#include "ServerManager.hpp"
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/event.h>
#include <sstream>
#include <iterator>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>

Connection::Connection(int clinet_socket_fd, sockaddr_in client_socket_addr, Config* config_ptr)
: request(), response(), cgi()
{
	this->client_socket_fd = clinet_socket_fd;
	this->client_socket_addr = client_socket_addr;
	this->config_ptr = config_ptr;
	this->progress = FROM_CLIENT;

}

Connection::Connection(const Connection& ref)
{
	this->client_socket_fd = ref.client_socket_fd;
	this->client_socket_addr = ref.client_socket_addr;
	this->config_ptr = ref.config_ptr;
	this->progress = ref.progress;
}

Connection::~Connection()
{

}

Connection&	Connection::operator=(const Connection& ref)
{
	(void)ref;
	return (*this);
}

void	Connection::mainprocess(struct kevent& event)
{
	std::cout << progress << event.filter << "\n";
	if (progress == FROM_CLIENT && event.filter == EVFILT_READ)
	{
		readClient();
		std::cout << "read done";
		makeResponse();
		if (progress == TO_CLIENT)
		{
			std::cout << "do combine\n";
			response.combineMessage();
			ServerManager::GetInstance().AddWriteEvent(client_socket_fd);
		}
	}
	else if (progress == TO_CGI && event.filter == EVFILT_WRITE)
		sendCgi();
	else if (progress == FROM_CGI && event.filter == EVFILT_READ)
		readCgi();
	else if (progress == FROM_FILE && event.filter == EVFILT_READ)
		readFile();
	else if (progress == TO_CLIENT && event.filter == EVFILT_WRITE)
	{
		std::cout << "sendMessage\n";
		sendMessage();
	}
}

void	Connection::readClient()
{
	char				buffer[1000000];
	std::cout << client_socket_fd << "\n";
	ssize_t				nread = read(client_socket_fd, buffer, sizeof(buffer));

	if (nread <= 0)
		ServerManager::GetInstance().CloseConnection(client_socket_fd);
	else
	{
		std::cout << "\n\n원본 메시지\n" << std::string(buffer, nread) << "\n\n\n";
		Parser	pars_buf(std::string(buffer, nread));
		pars_buf.ParseStartline(request);
		pars_buf.ParseHeader(request);
		pars_buf.ParseBody(request);
		pars_buf.ParseTrailer(request);
		std::cout << "파싱 메시지\n";
		std::cout << request.GetMethod() << " " << request.GetUrl() << " " << request.GetVersion() << '\n';
		std::map<std::string, std::string> tmp_map = request.GetHeader();
		for (std::map<std::string, std::string>::iterator it = tmp_map.begin(); it != tmp_map.end(); ++it)
			std::cout << it->first << ": \'" << it->second << "\'\n";
		std::cout << '\'' << request.GetBody() << "\'\n";
		std::cout << "status = " << request.GetStatus() << '\n';
	}
}

void	Connection::makeResponse()
{
	//request 유효성 검사
	//http/1.1 인데 host 헤더가 없을 때
	if (request.GetVersion() == "HTTP/1.1")
	{
		if (request.GetHeader().find("host") == request.GetHeader().end())
		{
			std::cout << "cant find host header\n";
			response.make_response_40x(400);
			progress = TO_CLIENT;
			return ;
		}
	}
	//request에 맞는 server 찾기
	server_ptr = &config_ptr->GetServerVec().front();
	std::cout << server_ptr << "\n";
	// for (std::vector<const Server>::iterator iter = config_ptr->GetServerVec().begin(); iter != config_ptr->GetServerVec().end(); iter++)
	// {
	// 	if (std::find(request.GetHeader().begin(), request.GetHeader().end(), iter->GetServerName()) != request.GetHeader().end())
	// 	{
	// 		server_ptr = &(*iter);
	// 		break ;
	// 	}
	// }
	//request에 맞는 location 찾기
	locate_ptr = NULL;
	const std::vector<Locate>& locate_vec = server_ptr->GetLocateVec();
	size_t	longest = 0;
	for (size_t i = 0; i < locate_vec.size(); ++i)
	{
		const std::string& s = request.GetUrl();
		std::cout << "url: " << request.GetUrl() << "\n";
		std::cout << "findurl: " << locate_vec[i].GetLocatePath() << "\n";
		if (s.find(locate_vec[i].GetLocatePath()) != std::string::npos)
		{
			if (locate_vec[i].GetLocatePath().size() > longest)
			{
				longest = locate_vec[i].GetLocatePath().size();
				locate_ptr = &locate_vec[i];
				std::cout << "longest: " << longest << "\n";
				std::cout << "url: " << request.GetUrl() << "\n";
			}
		}
	}
	if (locate_ptr == NULL)
	{
		std::cout << "cant find locate\n";
		response.make_response_40x(404);
		progress = TO_CLIENT;
		return ;
	}
	//request 유효성 검사끝


	//method 종류를 지원하는지 확인
	if (request.GetMethod() == OTHER)
	{
		std::cout << "method not defined\n";
		response.make_response_50x(501);
		progress = TO_CLIENT;
		return ;
	}

	//method 가 해당 로케이션에서 쓸 수 있는지 확인
	const std::vector<enum Method>& vec = locate_ptr->GetMethodVec();
	int flag = 0;
	for (size_t i = 0; i < vec.size(); i++)
	{
		if(vec[i] == request.GetMethod())
		{
			flag = 1;
			break ;
		}
	}
	if (flag == 0)
	{
		std::cout << "cant find allowd method\n";
		response.make_response_40x(403);
		progress = TO_CLIENT;
		return ;
	}

	//location에 리다이렉션 (return문)이 있는지 확인
	if (!locate_ptr->GetRedirectPair().second.empty())
	{
		int code = locate_ptr->GetRedirectPair().first;
		response.make_response_30x(code);
		std::cout << "redirected with" << code << "\n";
		progress = TO_CLIENT;
		return ;
	}

	//filepath 만들기
	path = "";
	path += locate_ptr->GetRoot();
	path += request.GetUrl();
	std::cout << "path: " << path << "\n";
	

	//file, dir, cgi 판단 및 분기
	if (path.back() == '/')
	{
		processDir();
		return ;
	}
	std::vector<std::string> cgivec = utils::SplitToVector(path, '/');
	if (cgivec.back() == server_ptr->GetCgiType())
	{
		processCgi();
		return ;
	}
	else
	{
		processFile();
		return ;
	}
}

void	Connection::processDir()
{
	//if method != GET
	if (request.GetMethod() != GET)
	{
		response.make_response_40x(405);
		return ;
	}
	//if request as directory but it is file
	struct stat buf;
	if (stat(path.c_str(), &buf) == -1)
	{
		response.make_response_40x(404);
		return;
	}
	//check if default index file exist
	if (!locate_ptr->GetIndexVec().empty() && locate_ptr->GetIndexVec().front() != "")
	{
		const std::vector<std::string> &index_vec = locate_ptr->GetIndexVec();
		for (size_t i = 0; i < index_vec.size(); i++)
		{
			std::string temp = path;
			temp += index_vec[i];
			struct stat buf2;
			if (stat(temp.c_str(), &buf2) != -1)
			{
				path = temp;
				processFile();
				return ;
			}
		}
	}
	//check if autoindex is on
	if (locate_ptr->GetAutoIndex())
	{
		// response.autoindex();
		return ;
	}
	else
	{
		response.make_response_40x(403);
		return ;
	}
}

void	Connection::processFile()
{
	//check if file exist
	struct stat buf;
	if (stat(path.c_str(), &buf) == -1)
	{
		response.make_response_40x(404);
		return ;
	}
	//request as file but is directory
	if (S_ISDIR(buf.st_mode))
	{
		response.make_response_30x(301);
		return ;
	}

	file_fd = open(path.c_str(), std::ios::binary);
	if (file_fd == -1)
	{
		response.make_response_40x(403);
		return ;
	}
	//파일 non-block 설정
	int flag = fcntl(file_fd, F_GETFL, 0);
	fcntl(file_fd, F_SETFL, flag | O_NONBLOCK);

	//파일 디스크립터와 connection 개체 map 추가
	ServerManager::GetInstance().AddConnectionMap(file_fd, *this);
	progress = FROM_FILE;
}

void	Connection::sendMessage()
{
	const std::string &buffer = response.getMessage();
	std::cout << "messagesize: " << response.getMessage().size() << "\n";
	ssize_t send_size = write(client_socket_fd, buffer.data(), response.getMessageSize() - 1);
	if (send_size < 0)
	{
		ServerManager::GetInstance().CloseConnection(client_socket_fd);
		return ;
	}
	else if (send_size == response.getMessageSize())
	{
		//response 완료
		ServerManager::GetInstance().CloseConnection(client_socket_fd);
		return ;
	}
	else
	{
		//send_size만큼 message에서 지우기
		response.cutMessage(send_size);
		return ;
	}
}

void	Connection::processCgi()
{
	//check if file exist
	struct stat tmp;
	if (stat(path.c_str(), &tmp) == -1)
	{
		response.make_response_40x(404);
		return ;
	}
	//환경변수 설정
	cgi.setEnv(request, *server_ptr);

	//pipe 설정
	cgi.setPipe();

	//cgi 실행
	if (cgi.CgiExec().ok())
	{
		ServerManager::GetInstance().AddConnectionMap(cgi.GetPipeIn()[1], *this);
		ServerManager::GetInstance().AddConnectionMap(cgi.GetPipeOut()[0], *this);
		progress = TO_CGI;
	}
	else
	{
		response.make_response_50x(500);
		ServerManager::GetInstance().AddConnectionMap(client_socket_fd, *this);
		progress = TO_CLIENT;
	}

}

void	Connection::readCgi()
{
	std::string buff;
	ssize_t	maxsize = 65535;
	buff.resize(maxsize);
	ssize_t readsize = read(cgi_output_fd, &buff[0], maxsize);
	if (readsize < 0)
	{
		response.make_response_50x(500);
		return ;
	}
	else if (readsize == maxsize)
	{
		response.addBody(buff, readsize);
		return ;
	}
	else
	{
		response.addBody(buff, readsize);
		ServerManager::GetInstance().CloseConnection(cgi_output_fd);
		ServerManager::GetInstance().AddConnectionMap(client_socket_fd, *this);
		progress = TO_CLIENT;
		return ;
	}
}

void	Connection::sendCgi()
{
	std::string buff;
	ssize_t	maxsize = 65535;
	buff.resize(maxsize);
	ssize_t writesize = write(cgi_input_fd, request.GetBody().data(), maxsize);
	if (writesize < 0)
	{
		response.make_response_50x(500);
		return ;
	}
	else if (writesize == maxsize)
	{
		request.cutbody(writesize);
		return ;
	}
	else
	{
		ServerManager::GetInstance().CloseConnection(cgi_input_fd);
		return ;
	}
}

void	Connection::readFile()
{
	std::string buff;
	ssize_t maxsize = server_ptr->GetClientBodySize();
	buff.resize(maxsize);
	ssize_t readsize = read(file_fd, &buff[0], maxsize);
	if (readsize < 0)
	{
		response.make_response_50x(500);
		return ;
	}
	else if (readsize == maxsize)
	{
		response.addBody(buff, readsize);
		return ;
	}
	else
	{
		response.addBody(buff, readsize);
		ServerManager::GetInstance().RemoveConnectionMap(file_fd);
		ServerManager::GetInstance().AddConnectionMap(client_socket_fd, *this);
		progress = TO_CLIENT;
		return ;
	}
}

int		Connection::GetClientSocketFd()
{
	return client_socket_fd;
}
