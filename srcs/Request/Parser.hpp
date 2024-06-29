#ifndef PARSER_H
# define PARSER_H

# include "Request.hpp"

class Request;

class Parser
{
public:
	// OCCF
	Parser(const std::string &buf);
	~Parser();

	// method
	void	ParseStartline(Request &request);
	void	ParseHeader(Request &request);
	void	ParseBody(Request &request);
	void	ParseTrailer(Request &request);

private:
	std::string	data;
	// forbidden method
	Parser();
	Parser(const Parser& copy);
	Parser& operator=(const Parser& rhs);
};

#endif
