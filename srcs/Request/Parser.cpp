#include "Parser.hpp"

Parser::Parser(const std::string &buf) : data(buf) {};

Parser::~Parser(){};

bool	Parser::parse_startline(Request &request){
	std::string	method;
	std::string	url;
	std::string	version;

	data >> method;
	if (method == "GET")
		request.set_method(GET);
	else if (method == "POST")
		request.set_method(POST);
	else if (method == "DELETE")
		request.set_method(DELETE);
	else
		request.set_method(OTHER);
	data >> url;
	request.set_url(url);
	data >> version;
	request.set_version(version);
	return true; // 추후 에러 처리가 불필요할 경우 제거, 반환형 void로 변경
};

bool	Parser::parse_header(Request &request){
	std::string	tmp_str;
	std::string	header_name;
	std::string	header_value;

	while (std::getline(data, tmp_str) && !tmp_str.empty())
	{
		if (tmp_str[tmp_str.size() - 1] == '\r')
			tmp_str.erase(tmp_str.size() - 1);
		
		
		// std::getline(tmp_stream, header_name, ':');
		// std::getline(tmp_stream, header_value);
	}
};

bool	Parser::parse_body(Request &request){
	std::string	tmp_str;
	std::string	body;
	while (std::getline(data, tmp_str) && !tmp_str.empty())
	{
		if (!body.empty())
			body += "\n";
		body += tmp_str;
	}
};

// definition of forbidden method
Parser::Parser(){};
Parser::Parser(const Parser& copy){};
Parser& Parser::operator=(const Parser& rhs){};