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
	this->file_fd = 0;
	this->pipein = 0;
	this->pipeout = 0;
	std::time(&timeval);
}

Connection::Connection(const Connection& ref)
: request(), response(), cgi()
{
	this->client_socket_fd = ref.client_socket_fd;
	this->client_socket_addr = ref.client_socket_addr;
	this->config_ptr = ref.config_ptr;
	this->progress = ref.progress;
	this->file_fd = ref.file_fd;
	this->pipein = ref.pipein;
	this->pipeout = ref.pipeout;
	this->timeval = ref.timeval;
}

Connection::~Connection()
{

}

Connection&	Connection::operator=(const Connection& ref)
{
	this->client_socket_fd = ref.client_socket_fd;
	this->client_socket_addr = ref.client_socket_addr;
	this->config_ptr = ref.config_ptr;
	this->progress = ref.progress;
	this->file_fd = ref.file_fd;
	this->pipein = ref.pipein;
	this->pipeout = ref.pipeout;
	this->timeval = ref.timeval;
	this->request = ref.request;
	this->cgi = ref.cgi;
	return (*this);
}



void	Connection::MainProcess(struct kevent& event)
{
	std::cout << "progress: " << progress << " event.filter: " << event.filter << "\n";
	std::cout << "MainProcess for fd: " << client_socket_fd << "\n";
	std::cout << "connection progress: " << progress << "\n";
	if (progress == FROM_CLIENT && event.filter == EVFILT_READ)
	{
		ReadClient();
		if (progress == END_CONNECTION)
			return ;
		if (progress == READ_CONTINUE)
		{
			progress = FROM_CLIENT;
			return ;
		}
		// progress == readcontinue면 from_client로 바꾸고 리턴 -> 이벤트와 fd 유지하고 계속 읽기 가능!
		std::cout << "read done";
		MakeResponse();
		response.CombineMessage();
		return ;
	}
	else if (progress == TO_CGI && event.filter == EVFILT_WRITE)
	{
		SendCgi();
		return ;
	}
	else if (progress == FROM_CGI && event.filter == EVFILT_READ)
	{
		ReadCgi();
		response.CombineMessage();
		return ;
	}
	else if (progress == FROM_FILE && event.filter == EVFILT_READ)
	{
		ReadFile();
		response.CombineMessage();
		return ;
	}
	else if (progress == TO_CLIENT && event.filter == EVFILT_WRITE)
	{
		// 여기서 쿠키 관련 작업?
		std::cout << "SendMessage\n";
		SendMessage();
		return ;
	}
	else
	{
		progress = END_CONNECTION;
		return ;
	}
}

void	Connection::ReadClient()
{
	char				buffer[1000000];
	std::cout << client_socket_fd << "\n";
	ssize_t				nread = read(client_socket_fd, buffer, sizeof(buffer));

	if (nread <= 0)
	{
		progress = END_CONNECTION;
		return ;
	}
	else
	{
		std::cout << "\n\n원본 메시지\n" << std::string(buffer, nread) << "\n\n\n";
		Parser	pars_buf(std::string(buffer, nread));
		pars_buf.ParseStartline(request);
		pars_buf.ParseHeader(request);
		if (request.GetStatus() == READ_BODY && (request.GetMethod() == GET || request.GetMethod() == DELETE))
			request.SetStatus(READ_DONE);
		pars_buf.ParseBody(request);
		pars_buf.ParseTrailer(request);
		std::cout << "파싱 메시지\n";
		std::cout << request.GetMethod() << " " << request.GetUrl() << " " << request.GetVersion() << '\n';
		std::map<std::string, std::string> tmp_map = request.GetHeader();
		for (std::map<std::string, std::string>::iterator it = tmp_map.begin(); it != tmp_map.end(); ++it)
			std::cout << it->first << ": \'" << it->second << "\'\n";
		std::cout << '\'' << request.GetBody() << "\'\n";

		std::cout << "status = " << request.GetStatus() << '\n';
		if (request.GetStatus() != READ_DONE)
			progress = READ_CONTINUE;
		return ;
	}
}

void	Connection::MakeResponse()
{
	//request 유효성 검사
	//http/1.1 인데 host 헤더가 없을 때
	if (request.GetStatus() == BAD_REQUEST)
	{
		response.make_response_40x(405);
		progress = TO_CLIENT;
		return ;
	}
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
	if (vec.size() != 0)
	{
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
	}

	//location에 리다이렉션 (return문)이 있는지 확인
	if (!locate_ptr->GetRedirectPair().second.empty())
	{
		int code = locate_ptr->GetRedirectPair().first;
		response.make_response_30x(code, locate_ptr->GetRedirectPair().second);
		std::cout << "redirected with" << code << "\n";
		progress = TO_CLIENT;
		return ;
	}

	//filepath 만들기
	path = "";
	path += locate_ptr->GetRoot();
	path += "/";
	path += request.GetUrl().substr(locate_ptr->GetLocatePath().size());
	std::cout << "path: " << path << "\n";
	

	//file, dir, cgi 판단 및 분기
	if (path.back() == '/')
	{
		ProcessDir();
		return ;
	}
	std::vector<std::string> cgivec = utils::SplitToVector(path, '/');
	std::cout << cgivec.back() << server_ptr->GetCgiType() << "\n";
	if (cgivec.back().find(server_ptr->GetCgiType()) != std::string::npos)
	{
		ProcessCgi();
		return ;
	}
	else
	{
		ProcessFile();
		return ;
	}
}

void	Connection::ProcessDir()
{
	//if method != GET
	std::cout << "ProcessDir\n";
	if (request.GetMethod() != GET)
	{
		response.make_response_40x(405);
		progress = TO_CLIENT;
		return ;
	}
	//if request as directory but it is file
	struct stat buf;
	if (stat(path.c_str(), &buf) == -1)
	{
		std::cout << "path: " << path << "\n";
		std::cout << "file not exist\n";
		response.make_response_40x(404);
		progress = TO_CLIENT;
		return;
	}
	if ((buf.st_mode & R_OK) == 0)
	{
		response.make_response_40x(403);
		progress = TO_CLIENT;
		return ;
	}
	//check if default index file exist
	if (!locate_ptr->GetIndexVec().empty())
	{
		const std::vector<std::string> &index_vec = locate_ptr->GetIndexVec();
		std::cout << index_vec.size() << "\n";
		for (size_t i = 0; i < index_vec.size(); i++)
		{
			std::string temp = path;
			temp += index_vec[i];
			struct stat buf2;
			if (stat(temp.c_str(), &buf2) != -1)
			{
				std::cout << "indexpath: " << temp << "\n";
				path = temp;
				ProcessFile();
				return ;
			}
		}
	}
	//check if autoindex is on
	if (locate_ptr->GetAutoIndex())
	{
		response.AutoIndex(path);
		progress = TO_CLIENT;
		return ;
	}
	else
	{
		response.make_response_40x(403);
		progress = TO_CLIENT;
		return ;
	}
}

void	Connection::ProcessFile()
{
	std::cout << "ProcessFile\n";

	//check if file exist
	struct stat buf;
	if (stat(path.c_str(), &buf) == -1)
	{
		std::cout << "ProcessFile: open failed\n";
		response.make_response_40x(404);
		progress = TO_CLIENT;
		return ;
	}
	//request as file but is directory
	if (S_ISDIR(buf.st_mode))
	{
		std::cout << "ProcessFile: is dir\n";
		response.make_response_30x(301, request.GetUrl() + "/");
		progress = TO_CLIENT;
		return ;
	}

	file_fd = open(path.c_str(), std::ios::binary);
	if (file_fd == -1)
	{
		std::cout << "ProcessFile: open file failed\n";
		response.make_response_40x(403);
		progress = TO_CLIENT;
		return ;
	}
	//파일 non-block 설정
	int flag = fcntl(file_fd, F_GETFL, 0);
	fcntl(file_fd, F_SETFL, flag | O_NONBLOCK);

	progress = FROM_FILE;
}

void	Connection::SendMessage()
{
	const std::string &buffer = response.GetMessage();
	std::cout << "---------message-------------\n";
	std::cout << response.GetMessage();
	std::cout << "---------message end-------------\n";
	std::cout << "messagesize: " << buffer.size() << "\n";
	ssize_t send_size = write(client_socket_fd, buffer.data(), buffer.size());
	std::cout << "send_size = " << send_size << "\n";
	if (send_size < 0)
	{
		std::cout << "read error\n";
		progress = END_CONNECTION;
		return ;
	}
	else if (send_size == response.GetMessageSize())
	{
		//response 완료
		progress = END_CONNECTION;
		return ;
	}
	else
	{
		//send_size만큼 message에서 지우기
		response.CutMessage(send_size);
		return ;
	}
}

void	Connection::ProcessCgi()
{
	std::cout << "ProcessCgi\n";
	//check if file exist
	struct stat tmp;
	std::cout << "path: " << path << "\n";
	if (stat(path.c_str(), &tmp) == -1)
	{
		std::cout << "cannot find path\n";
		response.make_response_40x(404);
		progress = TO_CLIENT;
		return ;
	}
	//환경변수 설정
	cgi.setEnv(request, *server_ptr);
	std::cout << "setenv setup done\n";

	//pipe 설정
	cgi.setPipe();
	std::cout << "pipe setup done\n";

	//cgi 실행
	if (cgi.CgiExec(path).ok())
	{
		pipein = cgi.GetPipeIn();
		pipeout = cgi.GetPipeOut();
		std::cout << "cgi setup done\n";
		progress = TO_CGI;
	}
	else
	{
		response.make_response_50x(500);
		progress = TO_CLIENT;
	}
}

void	Connection::ReadCgi()
{
	std::string buff;
	ssize_t	maxsize = 65535;
	buff.resize(maxsize);
	ssize_t readsize = read(pipein, &buff[0], maxsize);
	if (readsize < 0)
	{
		std::cout << "readcgi error\n";
		response.make_response_50x(500);
		progress = TO_CLIENT;
		return ;
	}
	else if (readsize == maxsize)
	{
		std::cout << "readcgi again << "<<readsize<<"\n";
		response.AddBody(buff, readsize);
		return ;
	}
	else
	{
		std::cout << "readcgi done << "<<readsize<<"\n";
		response.AddBody(buff, readsize);
		progress = TO_CLIENT;
		return ;
	}
}

void	Connection::SendCgi()
{
	std::cout << "SendCgi\n";
	std::string buff;
	ssize_t	maxsize = 65535;
	buff.resize(maxsize);
	ssize_t writesize = write(pipeout, request.GetBody().data(), request.GetBody().size());
	if (writesize < 0)
	{
		std::cout << "send cgi error\n";
		std::cout << errno << "\n";
		response.make_response_50x(500);
		progress = TO_CLIENT;
		return ;
	}
	else if (writesize == maxsize)
	{
		std::cout << "send cgi again << "<<writesize<<"\n";
		request.CutBody(writesize);
		return ;
	}
	else
	{
		std::cout << "send cgi done << "<<writesize<<"\n";
		progress = FROM_CGI;
		return ;
	}
}

void	Connection::ReadFile()
{
	std::string buff;
	ssize_t maxsize = server_ptr->GetClientBodySize();
	buff.resize(maxsize);
	ssize_t readsize = read(file_fd, &buff[0], maxsize);
	if (readsize < 0)
	{
		response.make_response_50x(500);
		progress = TO_CLIENT;
		return ;
	}
	else if (readsize == maxsize)
	{
		response.AddBody(buff, readsize);
		return ;
	}
	else
	{
		response.AddBody(buff, readsize);
		progress = TO_CLIENT;
		std::cout << readsize << ": ReadFile done\n";
		return ;
	}
}

int		Connection::GetClientSocketFd()
{
	return client_socket_fd;
}

enum CurrentProgress		Connection::GetProgress()
{
	return progress;
}

void		Connection::SetProgress(enum CurrentProgress progress)
{
	this->progress = progress;
}

int		Connection::GetFileFd()
{
	return file_fd;
}
int		Connection::GetPipein()
{
	return pipein;
}

int		Connection::GetPipeout()
{
	return pipeout;
}

std::time_t		Connection::GetTimeval()
{
	return timeval;
}
