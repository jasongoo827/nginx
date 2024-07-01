#ifndef ENUM_HPP
# define ENUM_HPP

enum Method
{
	GET,
	POST,
	DELETE,
	OTHER
};

enum ConfigVar
{
	SOFTWARE_NAME = 1 << 0,
	SOFTWARE_VER = 1 << 1,
	HTTP_VER = 1 << 2,
	CGI_VER = 1 << 3
};

enum ServerVar
{
	LISTEN = 1 << 0,
	SERVER_NAME = 1 << 1,
	CLIENT_SIZE = 1 << 2,
	CGI_EXT = 1 << 3
};

enum LocateVar
{
	METHOD = 1 << 0,
	REDIRECT = 1 << 1,
	ROOT = 1 << 2,
	INDEX = 1 << 3,
	AUTOINDEX = 1 << 4,
	FILEPATH = 1 << 5
};


#endif