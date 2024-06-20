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

	Status 						ReadConfig(std::string& file);
	Status						ParseConfig(std::string& file);
	Status						ParseServerVariable(std::string& file, std::istringstream& iss);
	std::string 				ExtractServerBlock(std::istringstream& iss, std::string& first_line);
	void						PrintConfigInfo(void);

	const std::vector<Server>& 	GetServerVec(void) const;
	const std::string&			GetSoftwareName(void) const;
	const std::string&			GetSoftwareVer(void) const;
	const std::string&			GetHttpVer(void) const;
	const std::string&			GetCgiVer(void) const;

private:
	std::vector<Server>	server_vec;
	std::string			software_name;
	std::string			software_ver;
	std::string			http_ver;
	std::string			cgi_ver;
};

#endif