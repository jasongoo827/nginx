#ifndef SERVERMANAGER_HPP
# define SERVERMANAGER_HPP

# include "Enum.hpp"
# include <map>
# include <string>
# include <vector>
# include <fcntl.h>
# include <sys/socket.h>
# include <sys/event.h>
# include <arpa/inet.h>
# include <string.h>
# include <unistd.h>
# include <Connection.hpp>


class ServerManager
{
public:
	ServerManager();
	ServerManager(const ServerManager& ref);
	~ServerManager();
	ServerManager& operator=(const ServerManager& ref);

	static ServerManager&	GetInstance();
	bool					RunServer(Config* config);
	bool					InitServerAddress(sockaddr_in &addr_serv, int port);
	bool					InitServerSocket(int &sock_serv, sockaddr_in &addr_serv);
	bool					InitClientSocket(int &kq, int &sock_serv, struct ::kevent &change_event, \
									int &sock_client, sockaddr_in &addr_client, socklen_t addr_client_len);
	bool					InitKqueue(int &kq, int &sock_serv, struct ::kevent &change_event);
	bool					CheckEvent(int &kq, struct ::kevent *events, int &event_count, int &sock_serv);
	void					CloseAllConnection();
	void					CloseConnection(int &sock_client);
	void					AddConnectionMap(int, Connection&);
	void					RemoveConnectionMap(int fd);
	void					AddWriteEvent(int client_socket_fd);


private:
	static ServerManager 					servermanager;
	std::map<int, Connection*>				connectionmap;
	std::vector<Connection>					v_connection;
	int										kq;
	int										event_count;
	int										sock_serv;
	struct sockaddr_in						addr_serv;
};

#endif
