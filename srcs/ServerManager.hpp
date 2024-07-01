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

	bool			RunServer(Config* config);
	bool			InitServerAddress(sockaddr_in &addr_serv, int port);
	bool			InitServerSocket(int &sock_serv, sockaddr_in &addr_serv);
	bool			InitClientSocket(int &kq, int &sock_serv, struct ::kevent &change_event, \
							int &sock_client, sockaddr_in &addr_client, socklen_t addr_client_len);
	bool			InitKqueue(int &kq, int &sock_serv, struct ::kevent &change_event);
	bool			CheckEvent(int &kq, struct ::kevent *events, int &event_count, int &sock_serv);
	static void		CloseAllConnection(std::vector<int> &v_client_sockets, \
						std::vector<sockaddr_in> &v_addr_client, int &kq, int &sock_serv);
	static void		CloseConnection(struct ::kevent &change_event, std::vector<int> &v_sock_client, \
						std::vector<sockaddr_in> &v_addr_client, int &kq, int &sock_client, \
						struct sockaddr_in &addr_client);
	static void		AddConnectionMap(int, Connection&);
	static void		RemoveConnectionMap(int fd);

private:
	static std::map<int, Connection*>		connectionmap;
	static std::vector<Connection>			v_connection;
};

#endif
