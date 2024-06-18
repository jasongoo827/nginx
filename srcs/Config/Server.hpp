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

	Status 		ParseServerBlock(std::istringstream& iss, std::string& server_block);
	std::string	ExtractLocateBlock(std::string& server_block);
	void		PrintServerInfo(void);

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