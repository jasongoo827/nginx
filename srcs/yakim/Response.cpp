#include "Response.hpp"
#include "Utils.hpp"
#include <algorithm>

std::map<int, std::string>	Response::reasonmap;
// Response::reasonmap[100] = "Continue";
// {}
// 	{100, "Continue"},
// 	{101, "Switching Protocols"},
// 	{200, "OK"},
// 	{201, "Created"},
// 	{202, "Accepted"},
// 	{203, "Non-Authoritative Information"},
// 	{204, "No Content"},
// 	{205, "Reset Content"},
// 	{206, "Partial Content"},
// 	{300, "Multiple Choices"},
// 	{301, "Moved Permanently"},
// 	{302, "Found"},
// 	{303, "See Other"},
// 	{304, "Not Modified"},
// 	{305, "Use Proxy"},
// 	{307, "Temporary Redirect"},
// 	{400, "Bad Request"},
// 	{401, "Unauthorized"},
// 	{403, "Forbidden"},
// 	{404, "Not Found"},
// 	{405, "Method Not Allowed"},
// 	{406, "Not Acceptable"},
// 	{407, "Proxy Autentication Required"},
// 	{408, "Request Timeout"},
// 	{409, "Conflict"},
// 	{410, "Gone"},
// 	{411, "Length Required"},
// 	{412, "Precondition Failed"},
// 	{413, "Request Entity Too Large"},
// 	{414, "Request URI Too Long"},
// 	{415, "Unsupported Media Type"},
// 	{416, "Requested Range Not Satisfiable"},
// 	{417, "Expectation Failed"},
// 	{500, "Internal Server Error"},
// 	{501, "Bad Gateway"},
// 	{502, "Service Unavailable"},
// 	{503, "Gateway Timeout"},
// 	{504, "HTTP Version Not Supported"},
// };

//OCCF
Response::Response()
{

}

Response::Response(const Response& ref)
{
	(void)ref;
}

Response::~Response()
{

}

Response& Response::operator=(const Response& ref)
{
	(void)ref;
	return (*this);
}

//GETTER
enum Method	Response::get_method()
{
	return method;
}

int	Response::get_status()
{
	return status;
}

std::map<std::string, std::string>	Response::get_header()
{
	return header;
}

const std::string&	Response::getBody()
{
	return body;
}

const std::string&	Response::getMessage()
{
	return message;
}

ssize_t	Response::getMessageSize()
{
	return message.size();
}


void	Response::make_response_30x(int status)
{
	this->status = status;
	addBasicHeader();
}

void	Response::make_response_40x(int status)
{
	this->status = status;
	addBasicHeader();
}

void	Response::make_response_50x(int status)
{
	this->status = status;
	addBasicHeader();
}
	
void	Response::cutMessage(ssize_t size)
{
	if (size == 0)
		return ;
	message.erase(message.begin(), message.begin() + size - 1);
}

void	Response::addHeader(std::string key, std::string value)
{
	header[key] = value;
}

void	Response::addBasicHeader()
{
	header["Server"] = "nginx/0.1";//
	header["Date"] = utils::getTime();
	header["Connection"] = "close";
}

void	Response::combineMessage()
{
	std::stringstream ss;

	ss << "HTTP/1.1 " << status << " " << getReason(status) << "\r\n";
	std::map<std::string, std::string>::iterator iter;
	for (iter = header.begin(); iter != header.end(); ++iter)
	{
		ss << (*iter).first << ": " << (*iter).second << "\r\n";
	}
	if (!body.empty())
	{
		ss << "\r\n";
		ss << body;
		ss << "\r\n";
	}
	ss << "\r\n";
	message = ss.str();// stringstream 의 str 함수는 새롭게 string 객체를 복사해서 생성, 성능저하 이슈 있을수 있음.
	message_size = message.size();
}

void	Response::addBody(const std::string& str, ssize_t size)
{
	body += str.substr(0, size);
}

const std::string	Response::getReason(int status)
{
	if (reasonmap.find(status) != reasonmap.end())
		return (reasonmap[status]);
	return ("");
}
