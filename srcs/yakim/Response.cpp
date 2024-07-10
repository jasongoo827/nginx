#include "Response.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <dirent.h>
#include <sys/stat.h>

std::map<int, std::string>	Response::reasonmap;

void	InitializeReasonmap()
{
	Response::reasonmap[100] = "Continue";
	Response::reasonmap[101] = "Switching Protocols";
	Response::reasonmap[200] = "OK";
	Response::reasonmap[201] = "Created";
	Response::reasonmap[202] = "Accepted";
	Response::reasonmap[203] = "Non-Authoritative Information";
	Response::reasonmap[204] = "No Content";
	Response::reasonmap[205] = "Reset Content";
	Response::reasonmap[206] = "Partial Content";
	Response::reasonmap[300] = "Multiple Choices";
	Response::reasonmap[301] = "Moved Permanently";
	Response::reasonmap[302] = "Found";
	Response::reasonmap[303] = "See Other";
	Response::reasonmap[304] = "Not Modified";
	Response::reasonmap[305] = "Use Proxy";
	Response::reasonmap[307] = "Temporary Redirect";
	Response::reasonmap[400] = "Bad Request";
	Response::reasonmap[401] = "Unauthorized";
	Response::reasonmap[403] = "Forbidden";
	Response::reasonmap[404] = "Not Found";
	Response::reasonmap[405] = "Method Not Allowed";
	Response::reasonmap[406] = "Not Acceptable";
	Response::reasonmap[407] = "Proxy Autentication Required";
	Response::reasonmap[408] = "Request Timeout";
	Response::reasonmap[409] = "Conflict";
	Response::reasonmap[410] = "Gone";
	Response::reasonmap[411] = "Length Required";
	Response::reasonmap[412] = "Precondition Failed";
	Response::reasonmap[413] = "Request Entity Too Large";
	Response::reasonmap[414] = "Request URI Too Long";
	Response::reasonmap[415] = "Unsupported Media Type";
	Response::reasonmap[416] = "Requested Range Not Satisfiable";
	Response::reasonmap[417] = "Expectation Failed";
	Response::reasonmap[500] = "Internal Server Error";
	Response::reasonmap[501] = "Bad Gateway";
	Response::reasonmap[502] = "Service Unavailable";
	Response::reasonmap[503] = "Gateway Timeout";
	Response::reasonmap[504] = "HTTP Version Not Supported";
}

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

void	Response::SetStatus(int status)
{
	this->status = status;
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

// void	Response::AddCookieHeader()
// {
// 	// request에 Cookie 있는지 확인 -> 밖에서 확인
	
// 	// 있으면 해당 쿠키가 갖고 있는 data field 리턴
// 	// key=value; expire; Path =/; HttpOnly;

// 	// 없으면 key=value pair 생성, 나머지 값들도 생성;
// 	// value는 현재 시간은 ms로 받아와서 생성

// 	// Response header에 set-cookie 추가 - userId = ""
// 	// SessionID
// 	header["set-cookie"] = GenerateCookie();

// 	// session cookie는 expire 설정 X
// 	// cookie는 expire 설정 해주면 브라우저에서 알아서 처리
// }

// std::string Response::GenerateCookie(void)
// {
// 	std::string cookie = "";
// 	// key=value 생성
// 	cookie += "key=value;";
// 	// expire 생성 - 시간 설정 얼마나?
// 	cookie += "expire=";
// 	cookie += SetExpireDate();
// 	// path 생성
// 	cookie += "path=/;";
// 	// HttpOnly;
// 	cookie += "HttpOnly";
// 	return cookie;
// }

// std::string Response::SetExpireDate(void)
// {
//     std::time_t current_time = std::time(NULL);
// 	// 원하는 시간 설정 가능
//     std::time_t future_time = current_time + (15 * 60);
//     std::tm* time_info = std::gmtime(&future_time);

//     char buffer[100];
//     std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", time_info);
//     return buffer;
// }

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
