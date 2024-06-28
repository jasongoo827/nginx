#ifndef REQUEST_H
# define REQUEST_H

# include <iostream>
# include <string>
# include <vector>
# include <map>

enum Incomplete
{
	NO_ERROR,
	STARTLINE,
	WRONG_HEADER,
	INVALID_CHUNK,
	BODY_SIZE
};

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
	void	set_status(enum Incomplete type);

	enum Method	get_method();
	const std::string&	get_url();
	const std::string &	get_version();
	std::map<std::string, std::string>&	get_header();
	const std::string&	get_body();
	enum Incomplete	get_status();
	const std::string	find_value_in_header(const std::string &key);

private:
	enum Method							method;
	std::string							url;
	std::string							version;
	std::map<std::string, std::string>	header;
	std::string							body;
	enum Incomplete						status;
};

#endif
