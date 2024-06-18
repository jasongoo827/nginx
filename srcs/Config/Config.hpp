#ifndef CONFIG_HPP
# define CONFIG_HPP

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include "../Utils.hpp"
#include <stdlib.h>

class Server;
class Status;

class Config
{
public:
	Config();
	Config(const Config& other);
	Config& operator=(const Config& rhs);
	~Config();

	Status 		ReadConfig(std::string& file);
	bool		CheckExtension(std::string& file);
	Status		ParseConfig(std::string& file);
	std::string	ExtractServerBlock(std::string& file);
	void		PrintConfigInfo(void);

private:
	std::vector<Server>	server_vec;
	std::string			software_name;
	std::string			software_ver;
	std::string			http_ver;
	std::string			cgi_ver;
};

#endif