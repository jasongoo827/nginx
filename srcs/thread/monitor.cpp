#include "monitor.hpp"
#include "Connection.hpp"

Monitor::Monitor() {}

Monitor::~Monitor() { pthread_join(thread, NULL); delete mapptr; }

void Monitor::ExecuteMonitor(std::map<int, Connection*>& connectionmap)
{
	mapptr = new std::map<int, Connection*>(connectionmap);

	if (pthread_create(&this->thread, NULL, PrintMap, mapptr))
	{
		std::cerr << "thread create error\n";
		return ;
	}
}

void* PrintMap(void *arg)
{
	std::map<int, Connection*> connectionmap = *static_cast<std::map<int, Connection*>*>(arg);

	while (true)
	{
		std::cout << "******Monitor Thread Executed******\n";
		for (std::map<int, Connection*>::iterator it = connectionmap.begin(); it != connectionmap.end(); ++it)
		{
			std::cout << "fd: " << it->first << ", connection socket fd: " << it->second->GetClientSocketFd() << '\n';
		}
		usleep(10000000);
	}
}