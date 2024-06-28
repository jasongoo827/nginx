#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "Enum.hpp"
# include <map>
# include <string>

class Response
{
public:
	Response();
	Response(const Response& ref);
	~Response();
	Response& operator=(const Response& ref);

	void								make_response_30x(int);
	void								make_response_40x(int);
	void								make_response_50x(int);
	void								autoindex();

	enum Method							get_method();
	int									get_status();
	std::map<std::string, std::string>	get_header();
	const std::string&					getBody();
	const std::string&					getMessage();
	ssize_t								getMessageSize();
	const std::string&					getReason(int status);
	void								cutMessage(ssize_t size);
	void								addHeader(std::string key, std::string value);
	void								addBasicHeader();
	void								combineMessage();
	void								addBody(const std::string& str, ssize_t size);
	void								cutBody(ssize_t size);
	static std::map<int, std::string>	reasonmap;

private:
	enum Method							method;
	int									status;
	std::map<std::string, std::string>	header;
	std::string							body;
	std::string							message;
	ssize_t								message_size;
};

#endif
