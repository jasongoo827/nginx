#include "response.hpp"

//OCCF
Response::Response()
{

}

Response::Response(const Response& ref)
{

}

Response::~Response()
{

}

Response& Response::operator=(const Response& ref)
{

}

//GETTER
enum Method	Response::get_method()
{
	return method;
}

int	Response::get_status()
{
	return status;
}

std::map<std::string, std::string>	Response::get_header()
{
	return header;
}

std::string	Response::get_body()
{
	return body;
}

enum Transfer_type	Response::get_transfer_type()
{
	return transfer_type;
}

const std::string&	Response::getMessage()
{
	return message;
}

ssize_t	Response::getMessageSize()
{
	return message.size();
}


void	Response::make_response_30x(int status)
{
	this->status = status;

}

void	Response::make_response_40x(int status)
{
	this->status = status;
}

void	Response::make_response_50x(int status)
{
	this->status = status;
}
	
void	Response::cutMessage(ssize_t size)
{
	if (size == 0)
		return ;
	message.erase(message.begin(), message.begin() + size - 1);
}
