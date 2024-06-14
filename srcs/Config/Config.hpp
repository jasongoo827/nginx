#ifndef CONFIG_HPP
# define CONFIG_HPP

#include <iostream>
#include <vector>

class Server;
class Locate;

class Config
{
public:
	Config();
	Config(const Config& other);
	Config& operator=(const Config& rhs);
	~Config();

private:
	double				http_ver;
	double				cgi_ver;
	std::string			software_name;
	std::string			software_ver;
	std::vector<Server>	server_vec;
};

#endif