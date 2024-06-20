#include "Request.hpp"

Request::Request(){};

Request::Request(const Request &copy) : method(copy.method), url(copy.url), version(copy.version), header(copy.header), body(copy.body){};

Request& Request::operator=(const Request &rhs){
	if (this == &rhs)
		return *this;
	this->method = rhs.method;
	this->url = rhs.url;
	this->version = rhs.version;
	this->header = rhs.header;
	this->body = rhs.body;
	return *this;
};

Request::~Request(){};

void	Request::set_method(enum Method type){
	this->method = type;
};

void	Request::set_url(const std::string &url){
	this->url = url;
};

void	Request::set_version(const std::string &version){
	this->version = version;
};

void	Request::set_header(std::map<std::string, std::string> &header){
	this->header = header;
};

void	Request::set_body(const std::string &body){
	this->body = body;
};
