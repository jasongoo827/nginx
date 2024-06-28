#include "Request.hpp"

Request::Request(): status(NO_ERROR){};

Request::Request(const Request &copy) : method(copy.method), url(copy.url), version(copy.version), header(copy.header), body(copy.body), status(copy.status){};

Request& Request::operator=(const Request &rhs)
{
	if (this == &rhs)
		return *this;
	this->method = rhs.method;
	this->url = rhs.url;
	this->version = rhs.version;
	this->header = rhs.header;
	this->body = rhs.body;
	this->status = rhs.status;
	return *this;
};

Request::~Request(){};

void	Request::set_method(enum Method type)
{
	this->method = type;
};

void	Request::set_url(const std::string &url)
{
	this->url = url;
};

void	Request::set_version(const std::string &version)
{
	this->version = version;
};

void	Request::set_header(std::map<std::string, std::string> &header)
{
	this->header = header;
};

void	Request::set_body(const std::string &body)
{
	this->body = body;
};

void	Request::set_status(enum Incomplete type)
{
	this->status = type;
};

enum Method	Request::get_method()
{
	return this->method;
};

const std::string&	Request::get_url()
{
	return this->url;
};

const std::string &	Request::get_version()
{
	return this->version;
};

std::map<std::string, std::string>&	Request::get_header()
{
	return this->header;
};

const std::string&	Request::get_body()
{
	return this->body;
};

enum Incomplete	Request::get_status()
{
	return this->status;
};

const std::string	Request::find_value_in_header(const std::string &key)
{
	if (this->header.find(key) != this->header.end())
		return (this->header[key]);
	return ("");
}
