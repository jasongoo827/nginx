#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <map>
# include <string>

typedef enum Method
{
	GET,
	POST,
	DELETE
};

typedef enum Uri_type
{
	DIR,
	FILE,
	CGI
};

typedef enum Transfer_type
{
	SINGLE,
	CHUNKED
};

typedef enum current_progress
{
	REQUEST,
	TO_CGI,
	FROM_CGI,
	FILE_READING,
	MESSAGE_SEND
};

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

//getter
	enum Method							get_method();
	std::map<std::string, std::string>	get_header();
	std::string							get_body();
	enum Transfer_type					get_transfer_type();

private:
	enum Method							method;
	std::map<std::string, std::string>	header;
	std::string							body;
	enum Transfer_type					transfer_type;
};

#endif
