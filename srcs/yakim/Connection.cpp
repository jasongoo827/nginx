#include "Connection.hpp"
#include <sys/stat.h>
#include <sstream>
#include <iterator>
#include <fcntl.h>
#include <unistd.h>

Connection::Connection(int clinet_socket_fd, Config* config_ptr)
{
	this->client_socket_fd = clinet_socket_fd;
	this->config_ptr = config_ptr;

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

void	Connection::makeResponse()
{
	//request 유효성 검사
	//http/1.1 인데 host 헤더가 없을 때
	if (request.get_header().find("Host") == request.get_header().end())
	{
		response.make_response_40x(400);
		return ;
	}
	
	//request에 맞는 server 찾기
	// if ()
	// {
		server_ptr = &config_ptr->GetServerVec().front();
		for (std::vector<const Server>::iterator iter = config_ptr->GetServerVec().begin(); iter != config_ptr->GetServerVec().end(); iter++)
		{
			if (request.get_header().find(iter->GetServerName()) == 0)
			{
				server_ptr = &(*iter);
				break ;
			}
		}
	//request에 맞는 location 찾기
		locate_ptr = &(server_ptr->GetLocationVec().front());
		for (std::vector<const Locate>::iterator iter = server_ptr->GetLocationVec().begin(); iter != server_ptr->GetLocationVec().end(); iter++)
		{
			if (request.get_uri().find(iter->GetLocatePath()) == 0)
			{
				locate_ptr = &(*iter);
				break ;
			}
		}

	//request 유효성 검사끝


	//method 종류를 지원하는지 확인
	// if (request.get_method() == OTHER)
	// {
	// 	response.make_response_50x(501);
	// 	return ;
	// }

	//method 가 해당 로케이션에서 쓸 수 있는지 확인
	// if(request.get_method() == )
	// else
	// {
	// 	response.make_response_40x(403);
	// 	return ;
	// }

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
	path += request.get_uri();
	

	//file, dir, cgi 판단 및 분기
	if (path.back() == '/')
		processDir();
	else
		processFile();
	// filepath = ;
	// if (filepath == )
	// 	process_dir();
	// else if (filepath == )
	// 	process_file();
	// else if (filepath == )
	// 	process_cgi();
}

void	Connection::processDir()
{
	//해당 디렉토리에 default 파일이 있는지 확인
	if (!locate_ptr->GetIndexVec().empty() && locate_ptr->GetIndexVec().front() != "")
	{
		const std::vector<std::string> &index_vec = locate_ptr->GetIndexVec();
		for (size_t i = 0; i < index_vec.size(); i++)
		{
			std::string temp = path;
			temp += index_vec[i];
			struct stat buf;
			if (stat(temp.c_str(), &buf) != -1)
			{
				path = temp;
				processFile();
				return ;
			}
		}
	}
	if (locate_ptr->GetAutoIndex())
		response.autoindex();
	else
		response.make_response_40x(403);
}

void	Connection::processFile()
{
	//파일이 존재하는지 확인
	struct stat buf;
	if (stat(path.c_str(), &buf) == -1)
	{
		response.make_response_40x(404);
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
}

void	Connection::readFile()
{
	std::string buffer;
	buffer.resize(server_ptr->GetClientBodySize());
	ssize_t read_size = read(file_fd, buffer.data(), server_ptr->GetClientBodySize());
	if (read_size == -1)
	{
		response.make_response_50x(500);
		return ;
	}
	else if (read_size == 0)
	{

	}
	else if (read_size == server_ptr->GetClientBodySize())
	{
		response.make_response_40x(413);
		return ;
	}
}

void	Connection::sendMessage()
{
	const std::string &buffer = response.getMessage();
	ssize_t send_size = write(client_socket_fd, buffer.data(), response.getMessageSize());
	if (send_size == -1)
	{
		response.make_response_50x(500);
		return ;
	}
	else if (send_size == response.getMessageSize())
	{
		//response 완료
		closeConnection();
	}
	else
	{
		//send_size만큼 message에서 지우기
		response.cutMessage(send_size);
	}
}

void	Connection::processCgi()
{

}

void	Connection::readCgi()
{

}

void	Connection::sendCgi()
{
	cgi()
}
