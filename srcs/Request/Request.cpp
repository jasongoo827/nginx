#include "Request.hpp"

Request::Request(){

};

Request::Request(const Request &copy){

};

Request& Request::operator=(const Request &rhs){

};

Request::~Request(){

};

void	Request::set_method(enum Method type){
	this->method = type;
};

void	Request::set_url(const std::string &url){
	this->url = url;
};

void	Request::set_version(const std::string &version){
	this->version = version;
};

void	Request::insert_header(const std::string &key, std::vector<std::string> &value){
	header[key] = value;
	// value 벡터로 변경, key는 하나 그대로 가기
};

void	Request::set_body(const std::string &body){
	this->body = body;
};
