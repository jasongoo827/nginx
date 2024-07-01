#ifndef REQUEST_H
# define REQUEST_H

# include "Utils.hpp"

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
	void								SetMethod(std::string method);
	void								SetUrl(const std::string &url);
	void								SetVersion(const std::string &version);
	void								SetBody(const std::string &body);
	void								SetStatus(enum Incomplete type);

	enum Method							GetMethod();
	const std::string&					GetUrl();
	const std::string &					GetVersion();
	std::map<std::string, std::string>&	GetHeader();
	const std::string&					GetBody();
	enum Incomplete						GetStatus();
	const std::string					FindValueInHeader(const std::string &key);

private:
	enum Method							method;
	std::string							url;
	std::string							version;
	std::map<std::string, std::string>	header;
	std::string							body;
	enum Incomplete						status;
};

#endif
