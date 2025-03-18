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
: method(GET), status(200), body(""), message(""), message_size(0), message_pos(0)
{
	this->header.clear();
}

Response::Response(const Response& ref)
: method(ref.method), status(ref.status), body(ref.body), message(ref.message), message_size(ref.message_size), message_pos(ref.message_pos)
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
	this->message_pos = ref.message_pos;
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

std::map<std::string, std::string>&	Response::GetHeader()
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

size_t	Response::GetMessagePos()
{
	return (message_pos);
}

void	Response::AddMessagePos(ssize_t sendsize)
{
	message_pos += sendsize;
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
	body = "";
	MakeErrorBody(status);
}

void	Response::make_response_50x(int status)
{
	this->status = status;
	body = "";
	MakeErrorBody(status);
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

void	Response::BodyResize(ssize_t size)
{
	body.reserve(size);
	body = "";
}

void	Response::AddBasicHeader()
{
	header["Server"] = "nginx/0.1";//
	header["Date"] = utils::getTime();
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
	message = ss.str();
	message_size = message.size();
	message_pos = 0;
}

void	Response::AddBody(const char* buff, ssize_t size)
{
	body.append(buff, size);
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

void	Response::SplitBodyHeaderData()
{
	std::string							tmp_body = "";
	std::string							tmp_str = "";
	std::string							header_name = "";
	std::string							header_value = "";

	if (body.find("\r\n\r\n") != std::string::npos)
	{
		tmp_body = utils::DivideStrByDoubleCRLF(body);
		while (!tmp_body.empty())
		{
			tmp_str = utils::DivideStrByCRLF(tmp_body);
			if (tmp_str.empty())
				break;
			if (tmp_str[0] == ' ' || tmp_str[0] == '\t')
			{
				utils::TrimSpaceTap(tmp_str);
				header.rbegin()->second += " " + tmp_str;
			}
			else
				utils::SplitHeaderData(tmp_str, header_name, header_value);
			AddHeader(header_name, header_value);
		}
	}
};

void	Response::MakeErrorBody(int status)
{
	body = "";
	std::stringstream	str;
	str << "<html>\r\n<head>\r\n<title>";
	str << status;
	str << " " + GetReason(status);
	str << "</title>\r\n";
	str << "</head>\r\n<body>\r\n<header>\r\n<h1>";
	str << status;
	str << " " + GetReason(status);
	str << "</h1>\r\n</header>\r\n<main>\r\n<section>\r\n";
	str << "<h2>Oops! Something Went Wrong.</h2>\r\n<a href='/'>Go to Homepage</a>\r\n</section>\r\n</main>\r\n</body>\r\n</html>";
	body = str.str();
}

void    Response::Cleaner()
{
	method = GET;
	status = 200;
	header.clear();
	body = "";
	message = "";
	message_size = 0;
	message_pos = 0;
}
