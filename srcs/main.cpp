#include "ServerManager.hpp"
#include "Config.hpp"
#include "Status.hpp"
#include "Parser.hpp"
#include "Request.hpp"
#include "Response.hpp"

void	InitializeReasonmap();

int main(int argc, char *argv[])
{
	InitializeReasonmap();
	Config config;
	Status status;
	if (argc == 2)
	{
		std::string file(argv[1]);
		status = config.ReadConfig(file);
		if (!status.ok())
		{
			std::cerr << status.message() << '\n';
			return (1);
		}
	}
	else
	{
		std::string file("usr/config/nginx.conf");
		status = config.ReadConfig(file);
		if (!status.ok())
		{
			std::cerr << status.message() << '\n';
			return (1);
		}
	}
	// config.PrintConfigInfo();
	ServerManager	servermanager;
	servermanager.RunServer(&config);
}