#include "monitor.hpp"
#include "Connection.hpp"

Monitor::Monitor() { }

Monitor::~Monitor() 
{ 
	// pthread_join(thread, NULL); 
	// pthread_mutex_destroy(&cout_mutex);
	// delete mapptr; 
}

void Monitor::ExecuteMonitor(std::map<int, Connection*>& connectionmap)
{
	mapptr = new std::map<int, Connection*>(connectionmap);
	pthread_mutex_init(&cout_mutex, NULL);

	if (pthread_create(&this->thread, NULL, PrintMap, this))
	{
		std::cerr << "thread create error\n";
		return ;
	}
}

void* PrintMap(void *arg)
{
	Monitor* monitor = static_cast<Monitor*>(arg);
	std::map<int, Connection*> connectionmap = *monitor->mapptr;

	while (true)
	{
		pthread_mutex_lock(&monitor->cout_mutex);
		std::cout << "******Monitor Thread routine start******\n";
		for (std::map<int, Connection*>::iterator it = connectionmap.begin(); it != connectionmap.end(); ++it)
		{
			std::cout << "fd: " << it->first << ", connection socket fd: " << it->second->GetClientSocketFd() << '\n';
		}
		std::cout << "******Monitor Thread routine end******\n";
		pthread_mutex_unlock(&monitor->cout_mutex);
		usleep(10000000);
	}
}