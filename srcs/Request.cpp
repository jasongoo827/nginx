#include "Request.hpp"

Request::Request(): status(READ_STARTLINE), bytes_to_read(0)
{
	body_pos = 0;
};

Request::Request(const Request &copy) : method(copy.method), url(copy.url), version(copy.version), header(copy.header), body(copy.body), body_pos(copy.body_pos), status(copy.status), bytes_to_read(copy.bytes_to_read){};

Request& Request::operator=(const Request &rhs)
{
	if (this == &rhs)
		return *this;
	this->method = rhs.method;
	this->url = rhs.url;
	this->version = rhs.version;
	this->header = rhs.header;
	this->body = rhs.body;
	this->body_pos = rhs.body_pos;
	this->status = rhs.status;
	this->bytes_to_read = rhs.bytes_to_read;
	return *this;
}

Request::~Request(){};

void	Request::SetMethod(std::string method)
{
	if (method == "GET")
		this->method = GET;
	else if (method == "POST")
		this->method = POST;
	else if (method == "DELETE")
		this->method = DELETE;
	else if (method == "")
		this->method = EMPTY;
	else
		this->method = OTHER;
}

void	Request::SetUrl(const std::string &url)
{
	this->url = url;
}

void	Request::SetVersion(const std::string &version)
{
	this->version = version;
};

void	Request::SetBody(const std::string &body)
{
	this->body = body;
};

void	Request::SetStatus(enum Incomplete type)
{
	this->status = type;
};

void   Request::ReserveBody(ssize_t size)
{
	if (body.empty())
		body.reserve(size);
};

void	Request::SetBytesToRead(int bytes){
	this->bytes_to_read = bytes;
}

enum Method	Request::GetMethod()
{
	return this->method;
};

const std::string&	Request::GetUrl()
{
	return this->url;
};

const std::string &	Request::GetVersion()
{
	return this->version;
}

std::map<std::string, std::string>&	Request::GetHeader()
{
	return this->header;
}

std::string&	Request::GetBody()
{
	return this->body;
}

void	Request::CutBody(ssize_t size)
{
	this->body.erase(0, size);
}

enum Incomplete	Request::GetStatus()
{
	return this->status;
}

ssize_t	Request::GetBytesToRead(){
	return this->bytes_to_read;
}

size_t Request::GetBodyPos()
{
	return this->body_pos;
}

void	Request::AddBodyPos(size_t body_pos)
{
	this->body_pos += body_pos;
}

const std::string	Request::FindValueInHeader(const std::string &key)
{
	if (this->header.find(key) != this->header.end())
		return (this->header[key]);
	return ("");
}

void	Request::Cleaner()
{
	method = GET;
	url = "";
	version = "";
	header.clear();
	body = "";
	body_pos = 0;
	status = READ_STARTLINE;
	bytes_to_read = 0;
}
