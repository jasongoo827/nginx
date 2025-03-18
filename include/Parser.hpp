#ifndef PARSER_H
# define PARSER_H

# include "Request.hpp"

class Request;

class Parser
{
public:
	// OCCF
	Parser();
	~Parser();

	// method
	void			SetMaxBodySize(ssize_t size);
	std::string&	GetData();
	void			ParseStartline(Request &request);
	void			ParseHeader(Request &request);
	void			ParseBody(Request &request);
	void			ParseTrailer(Request &request);
	void			Cleaner();

private:
	ssize_t		max_body_size;
	std::string	data;
	// forbidden method
	Parser(const Parser& copy);
	Parser& operator=(const Parser& rhs);
};

#endif
