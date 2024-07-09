#include "Response.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <dirent.h>
#include <sys/stat.h>

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
: method(GET), status(200), body(""), message(""), message_size(0)
{
	this->header.clear();
}

Response::Response(const Response& ref)
: method(ref.method), status(ref.status), body(ref.body), message(ref.message), message_size(ref.message_size)
{
	header = ref.header;
}

Response::~Response()
{
	header.clear();
}

Response& Response::operator=(const Response& ref)
{
	this->method = ref.method;
	this->status = ref.status;
	this->header = ref.header;
	this->body = ref.body;
	this->message = ref.message;
	this->message_size = ref.message_size;
	return (*this);
}

//GETTER
enum Method	Response::GetMethod()
{
	return method;
}

int	Response::GetStatus()
{
	return status;
}

std::map<std::string, std::string>	Response::GetHeader()
{
	return header;
}

const std::string&	Response::GetBody()
{
	return body;
}

const std::string&	Response::GetMessage()
{
	return message;
}

ssize_t	Response::GetMessageSize()
{
	return message.size();
}


void	Response::make_response_30x(int status, std::string str)
{
	this->status = status;
	AddHeader("Location", str);
}

void	Response::make_response_40x(int status)
{
	this->status = status;
}

void	Response::make_response_50x(int status)
{
	this->status = status;
}
	
void	Response::CutMessage(ssize_t size)
{
	if (size == 0)
		return ;
	message.erase(message.begin(), message.begin() + size);
}

void	Response::AddHeader(std::string key, std::string value)
{
	header[key] = value;
}

void	Response::AddBasicHeader()
{
	header["Server"] = "nginx/0.1";//
	header["Date"] = utils::getTime();
	header["Connection"] = "close";
}

void	Response::CombineMessage()
{
	std::stringstream ss;

	ss << "HTTP/1.1 " << status << " " << GetReason(status) << "\r\n";
	AddBasicHeader();
	std::map<std::string, std::string>::iterator iter;
	for (iter = header.begin(); iter != header.end(); ++iter)
	{
		ss << (*iter).first << ": " << (*iter).second << "\r\n";
	}
	ss << "Content-Length: " << body.size() << "\r\n";
	ss << "\r\n";
	ss << body;
	message = ss.str();// stringstream 의 str 함수는 새롭게 string 객체를 복사해서 생성, 성능저하 이슈 있을수 있음.
	message_size = message.size();
}

void	Response::AddBody(const std::string& str, ssize_t size)
{
	body += str.substr(0, size);
}

const std::string	Response::GetReason(int status)
{
	if (reasonmap.find(status) != reasonmap.end())
		return (reasonmap[status]);
	return ("");
}

void Response::AutoIndex(const std::string& path)
{
	DIR* 				dirptr;
	struct dirent		*dp;
	struct stat			stat_;
	std::stringstream	str;
	std::string			tmp;

	std::cout << "AutoIndex for path: " << path << "\n";
	dirptr = opendir(path.c_str());
	if (dirptr == NULL)
	{
		make_response_50x(403);
		return ;
	}
	str << "<html>\r\n<head><title>Index of " << path.c_str() << "</title></head>\r\n";
	str << "<body>\r\n<h1>Index of " << path.c_str() << "</h1><hr>\r\n<pre>";
	while ((dp = readdir(dirptr)) != NULL)
	{
		if (strcmp(dp->d_name, ".") == 0)
			continue ;
		stat(dp->d_name, &stat_);
		tmp = "";
		str << "<a href=\"" << dp->d_name << "\">" << dp->d_name << std::left << "</a>";
		if (S_ISREG(stat_.st_mode))
		{
			std::string str_tmp = std::ctime(&stat_.st_mtime);
			str_tmp.pop_back();
			str << str_tmp << stat_.st_size;
		}
		str << "\r\n";
	}
	str << "</pre><hr></body>\r\n</html>";
	body = str.str();
	closedir(dirptr);
}
