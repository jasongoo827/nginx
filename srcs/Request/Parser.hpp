#ifndef PARSER_H
# define PARSER_H

# include "Request.hpp"
# include <sstream>

# define CHUNK_SIZE	0
# define CHUNK_DATA	1

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
	bool	parse_trailer(Request &request);

private:
	std::istringstream	data;
	// forbidden method
	Parser();
	Parser(const Parser& copy);
	Parser& operator=(const Parser& rhs);
};

#endif
