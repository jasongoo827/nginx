#ifndef SERVER_HPP
# define SERVER_HPP

#include <iostream>
#include <map>
#include <vector>
#include <sstream>
#include <stdlib.h>
#include "Utils.hpp"
#include "Enum.hpp"

class Locate;
class Status;

class Server
{
public:
	Server();
	Server(const Server& other);
	Server& operator=(const Server& rhs);
	~Server();

	Status 								ParseServerBlock(std::string& server_block);
	Status								ParsePortVariable(std::string& str);
	Status								ParseServerName(std::string& str);
	Status								ParseErrorPage(std::string& str);
	// Status								ParseCgiType(std::string& str);
	// Status								ParseClientSize(std::string& str);
	Status								ParseFilePath(std::string& str);
	Status								ParseLocateVariable(std::string& str, std::istringstream& iss);
	std::string							ExtractLocateBlock(std::istringstream& iss, std::string& first_line);
	void								PrintServerInfo(void);

	const std::vector<Locate>& 			GetLocateVec(void) const;
	const std::map<int, std::string>& 	GetErrorPage(void) const;
	const std::string& 					GetHostIp(void) const;
	const std::string& 					GetServerName(void) const;
	// const std::string& 					GetCgiType(void) const;
	// const std::vector<std::string>&		GetCgiVec(void) const;
	const ssize_t& 						GetPort(void) const;
	// const ssize_t& 						GetClientBodySize(void) const;
	const std::string& 					GetFilePath(void) const;

private:
	std::vector<Locate>			locate_vec;
	std::map<int, std::string> 	error_page;
	std::string					host_ip;
	std::string					server_name;
	// std::string 				cgi_type;
	// std::vector<std::string>	cgi_vec;
	ssize_t						port;
	// ssize_t						client_body_size;
	std::string					file_path;
	int							dup_mask;
};

#endif