#ifndef REQUEST_H
# define REQUEST_H

#include <iostream>
#include <string>
#include <vector>
#include <map>

enum Method
{
	GET,
	POST,
	DELETE,
	OTHER
};

class Request
{
public:
	// OCCF
	Request();
	Request(const Request &copy);
	Request& operator=(const Request &rhs);
	~Request();

	// method
	void	set_method(enum Method type);
	void	set_url(const std::string &url);
	void	set_version(const std::string &version);
	void	set_header(std::map<std::string, std::string> &header);
	void	set_body(const std::string &body);

private:
	enum Method							method;
	std::string							url;
	std::string							version;
	std::map<std::string, std::string>	header;
	std::string							body;
};

#endif
