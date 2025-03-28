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

	void								make_response_30x(int, std::string str);
	void								make_response_40x(int);
	void								make_response_50x(int);
	void								MakeErrorBody(int status);

	enum Method							GetMethod();
	int									GetStatus();
	std::map<std::string, std::string>&	GetHeader();
	const std::string&					GetBody();
	const std::string&					GetMessage();
	ssize_t								GetMessageSize();
	size_t								GetMessagePos();
	void								AddMessagePos(ssize_t sendsize);
	const std::string					GetReason(int status);
	void								SetStatus(int status);

	void								BodyResize(ssize_t size);

	void								CutMessage(ssize_t size);
	void								AddHeader(std::string key, std::string value);
	void								AddBasicHeader();
	void								AddCookieHeader();
	std::string							GenerateCookie(void);
	std::string							SetExpireDate(void);
	void								CombineMessage();
	void								AddBody(const char* buff, ssize_t size);
	void								CutBody(ssize_t size);
	void								AutoIndex(const std::string& path);
	void								SplitBodyHeaderData();
	void    							Cleaner();

	static std::map<int, std::string>	reasonmap;

private:
	enum Method							method;
	int									status;
	std::map<std::string, std::string>	header;
	std::string							body;
	std::string							message;
	ssize_t								message_size;
	size_t								message_pos;
};

#endif
