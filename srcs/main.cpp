#include "ServerManager.hpp"
#include "Config.hpp"
#include "Status.hpp"
#include "Parser.hpp"
#include "Request.hpp"

int main(int argc, char *argv[], char *envp[])
{
	(void)envp;
	Config config;
	if (argc == 2)
	{
		std::string file(argv[1]);
		Status status = config.ReadConfig(file);
		if (!status.ok())
		{
			std::cerr << status.message() << '\n';
			return (1);
		}
		// config.PrintConfigInfo();
	}
	else
	{
		std::string file("usr/config/nginx.conf");
		Status status = config.ReadConfig(file);
		if (!status.ok())
		{
			std::cerr << status.message() << '\n';
			return (1);
		}
		config.PrintConfigInfo();
	}
	std::cout << "config done" << std::endl;
	ServerManager	servermanager;
	servermanager.RunServer(&config);
}