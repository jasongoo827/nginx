#include "response.hpp"

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

void	Response::make_response_30x(int status)
{

}

void	Response::make_response_40x(int status)
{

}

void	Response::make_response_50x(int status)
{
	
}

enum Method	Response::get_method()
{
	return method;
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
