#ifndef PARSER_H
# define PARSER_H

#include "Request.hpp"
#include <sstream>

class Request;

class Parser
{
public:
	// OCCF
	Parser(const std::string &buf);
	~Parser();

	// method
	bool	parse_startline(Request &request);
	bool	parse_header(Request &request);
	bool	parse_body(Request &request);

private:
	std::istringstream	data;
	// forbidden method
	Parser();
	Parser(const Parser& copy);
	Parser& operator=(const Parser& rhs);
};

#endif
