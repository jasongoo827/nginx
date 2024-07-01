#include "Connection.hpp"
#include "Utils.hpp"
#include "Server.hpp"
#include <sys/stat.h>
#include <sstream>
#include <iterator>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>

Connection::Connection(int clinet_socket_fd, Config* config_ptr)
: request(), response(), cgi()
{
	this->client_socket_fd = clinet_socket_fd;
	this->config_ptr = config_ptr;
	this->progress = FROM_CLIENT;

}

Connection::Connection(const Connection& ref)
{

}

Connection::~Connection()
{

}

Connection&	Connection::operator=(const Connection& ref)
{

}

void	Connection::mainprocess()
{
	if (progress == FROM_CLIENT)
	{
		readclient();
		makeResponse();
		if (response.get_status() == 200)
			;
	}
	else if (progress == TO_CGI)
		sendCgi();
	else if (progress == FROM_CGI)
		readCgi();
	else if (progress == FROM_FILE)
		readFile();
	else if (progress == TO_CLIENT)
		sendMessage();
}

void	Connection::makeResponse()
{
	//request 유효성 검사
	//http/1.1 인데 host 헤더가 없을 때
	if (request.get_version() == "HTTP/1.1")
	{
		if (request.get_header().find("Host") == request.get_header().end())
		{
			response.make_response_40x(400);
			return ;
		}
	}
	//request에 맞는 server 찾기
	server_ptr = &config_ptr->GetServerVec().front();
	for (std::vector<const Server>::iterator iter = config_ptr->GetServerVec().begin(); iter != config_ptr->GetServerVec().end(); iter++)
	{
		if (std::find(request.get_header().begin(), request.get_header().end(), iter->GetServerName()) != 0)
		{
			server_ptr = &(*iter);
			break ;
		}
	}
	//request에 맞는 location 찾기
	locate_ptr = NULL;
	const std::vector<Locate>& locate_vec = server_ptr->GetLocateVec();
	size_t	longest = 0;
	for (size_t i = 0; i < locate_vec.size(); ++i)
	{
		const std::string& s = request.get_url();
		if (s.find(locate_vec[i].GetRoot()) == 0)
		{
			if (locate_vec[i].GetRoot().size() > longest)
			{
				longest = locate_vec[i].GetRoot().size();
				locate_ptr = &locate_vec[i];
			}

		}
	}
	if (locate_ptr == NULL)
	{
		response.make_response_40x(404);
		return ;
	}
	//request 유효성 검사끝


	//method 종류를 지원하는지 확인
	if (request.get_method() == OTHER)
	{
		response.make_response_50x(501);
		return ;
	}

	//method 가 해당 로케이션에서 쓸 수 있는지 확인
	const std::vector<enum Method>& vec = locate_ptr->GetMethodVec();
	for (size_t i = 0; i < vec.size(); i++)
	{
		if(vec[i] == request.get_method())
			break ;

		response.make_response_40x(403);
		return ;
	}

	//location에 리다이렉션 (return문)이 있는지 확인
	if (!locate_ptr->GetRedirectPair().second.empty())
	{
		int code = locate_ptr->GetRedirectPair().first;
		response.make_response_30x(code);
		return ;
	}

	//filepath 만들기
	path = "";
	path += locate_ptr->GetRoot();
	path += request.get_url();
	

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
	if (request.get_method() != GET)
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
		response.autoindex();
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
		response.setBody(path + "/");
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
	ServerManager::addMap(file_fd, *this);
	progress = FROM_FILE;
}

void	Connection::sendMessage()
{
	const std::string &buffer = response.getMessage();
	ssize_t send_size = write(client_socket_fd, buffer.data(), response.getMessageSize());
	if (send_size < 0)
	{
		ServerManager::closeConnection(client_socket_fd);
		return ;
	}
	else if (send_size == response.getMessageSize())
	{
		//response 완료
		ServerManager::closeConnection(client_socket_fd);
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
	if (cgi.cgiExec())
	{
		progress = TO_CGI;
	}
	else
	{
		response.make_response_50x(500);
		ServerManager::addConnection(client_socket_fd);
		progress = TO_CLIENT;
	}

}

void	Connection::readCgi()
{
	std::string buff;
	ssize_t	maxsize = 65535;
	buff.resize(maxsize);
	size_t readsize = read(cgi_output_fd, &buff[0], maxsize);
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
		ServerManager::closeConnection(cgi_output_fd);
		ServerManager::addConnection(client_socket_fd);
		progress = TO_CLIENT;
		return ;
	}
}

void	Connection::sendCgi()
{
	std::string buff;
	ssize_t	maxsize = 65535;
	buff.resize(maxsize);
	size_t writesize = write(cgi_input_fd, request.get_body().data(), maxsize);
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
		ServerManager::closeConnection(cgi_input_fd);
		return ;
	}
}

void	Connection::readFile()
{
	std::string buff;
	ssize_t maxsize = server_ptr->GetClientBodySize();
	buff.resize(maxsize);
	size_t readsize = read(file_fd, &buff[0], maxsize);
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
		ServerManager::closeConnection(file_fd);
		ServerManager::addConnection(client_socket_fd);
		progress = TO_CLIENT;
		return ;
	}

}
