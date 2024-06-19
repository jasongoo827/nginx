#ifndef SERVER_HPP
# define SERVER_HPP

#include <iostream>
#include <map>
#include <vector>
#include <sstream>
#include "../Utils.hpp"

class Locate;
class Status;

class Server
{
public:
	Server();
	Server(const Server& other);
	Server& operator=(const Server& rhs);
	~Server();

	Status 						ParseServerBlock(std::string& server_block);
	Status						ParsePortVariable(std::string& str);
	Status						ParseErrorPage(std::string& str);
	Status						ParseClientSize(std::string& str);
	Status						ParseLocateVariable(std::string& str, std::istringstream& iss);
	std::string					ExtractLocateBlock(std::istringstream& iss, std::string& first_line);
	void						PrintServerInfo(void);

	std::vector<Locate> 		GetLocateVec(void) const;
	std::map<int, std::string> 	GetErrorPage(void) const;
	std::string 				GetHostIp(void) const;
	std::string 				GetServerName(void) const;
	std::string 				GetCgiType(void) const;
	int 						GetPort(void) const;
	int 						GetClientBodySize(void) const;

private:
	std::vector<Locate>			locate_vec;
	std::map<int, std::string> 	error_page;
	std::string					host_ip;
	std::string					server_name;
	std::string 				cgi_type; // 추후에 vector로 수정될 수 있음.
	int							port;
	int							client_body_size;

};

#endif