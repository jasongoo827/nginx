#include "Config/Config.hpp"
#include "Status.hpp"

int main(int argc, char *argv[], char *envp[])
{
	(void)envp;
	if (argc == 2)
	{
		std::string file(argv[1]);
		Config config;
		Status status = config.ReadConfig(file);
		if (!status.ok())
		{
			std::cerr << status.message() << '\n';
			return (1);
		}
		config.PrintConfigInfo();
	}
	else
	{
		// read default config file
	}
}