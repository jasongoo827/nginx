#ifndef REQUEST_H
# define REQUEST_H

# include "Utils.hpp"
# include "Enum.hpp"

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
	void								SetBytesToRead(int bytes);
	void								ReserveBody(ssize_t size);

	enum Method							GetMethod();
	const std::string&					GetUrl();
	const std::string &					GetVersion();
	std::map<std::string, std::string>&	GetHeader();
	std::string&						GetBody();
	size_t								GetBodyPos();
	enum Incomplete						GetStatus();
	ssize_t								GetBytesToRead();
	void								AddBodyPos(size_t body_pos);
	void								CutBody(ssize_t size);
	const std::string					FindValueInHeader(const std::string &key);
	void								Cleaner();

private:
	enum Method							method;
	std::string							url;
	std::string							version;
	std::map<std::string, std::string>	header;
	std::string							body;
	size_t								body_pos;
	enum Incomplete						status;
	ssize_t								bytes_to_read;
};

#endif
