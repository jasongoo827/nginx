#include "Connection.hpp"
#include <sys/stat.h>
#include <sstream>

Connection::Connection(int clinet_socket_fd, Config* config_ptr)
{
	this->client_socket_fd = clinet_socket_fd;
	this->config_ptr = config_ptr;

}

void	Connection::make_response()
{
	//request 유효성 검사
	//http/1.1 인데 host 헤더가 없을 때
	// if ()
	// {
	// 	response.make_response_40x(400);
	// 	return ;
	// }
	
	//request에 맞는 location 찾기
	// if ()
	// {

	// }
	// else
	// {
	// 	response.make_response_40x(405);
	// 	return ;
	// }

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
		response.make_response_40x(code);
		return ;
	}

	//file, dir, cgi 판단 및 분기
	// filepath = ;
	// if (filepath == )
	// 	process_dir();
	// else if (filepath == )
	// 	process_file();
	// else if (filepath == )
	// 	process_cgi();
}

void	Connection::process_dir()
{
	//해당 디렉토리에 default 파일이 있는지 확인
	// else
	{
		if (locate_ptr->GetAutoIndex())
			response.autoindex();
		else
			response.make_response_40x(403);
	}
}

void	Connection::process_file()
{
	std::ifstream infile("./images.jpeg", std::ios::binary);
	if (!infile.is_open())
	{
		response.make_response_40x(403);
		return ;
	}
	// infile.seekg(0, std::ios::end);
	// std::streampos fileSize = infile.tellg();
	// infile.seekg(0, std::ios::beg);
	// std::string buffer(fileSize);
	// infile.read(buffer.data(), fileSize);
	// std::string fileContents(buffer.begin(), buffer.end());
	// response.body = fileContents;
	// infile.close();
}

void	Connection::process_cgi()
{

}
