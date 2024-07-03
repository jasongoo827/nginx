#ifndef MONITOR_HPP
# define MONITOR_HPP

#include <iostream>
#include <pthread.h>
#include <map>
#include <unistd.h>

class Connection;

class Monitor
{
public:
	Monitor();
	~Monitor();

	void 		ExecuteMonitor(std::map<int, Connection*>& connectionmap);

private:
	pthread_t thread;
	std::map<int, Connection*>* mapptr;
};

void		*PrintMap(void *arg);

#endif