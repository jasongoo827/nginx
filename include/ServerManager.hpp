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
# include "Connection.hpp"
# include "Session.hpp"

class ServerManager
{
public:
	ServerManager();
	~ServerManager();

	bool					RunServer(Config* config);
	bool					CheckValidServer(size_t &sock);
	void					CloseAllServsock();
	bool					InitServerAddress(int &kq, sockaddr_in &addr_serv, int port);
	bool					InitServerSocket(int &kq, int &sock_serv, sockaddr_in &addr_serv);
	bool					InitClientSocket(int &kq, int &sock_serv, struct ::kevent &change_event, \
									int &sock_client, sockaddr_in &addr_client, socklen_t addr_client_len);
	bool					InitKqueue(int &kq, int &sock_serv);
	bool					RegistSockserv(int &kq, int &sock_serv, struct ::kevent &change_event);
	bool					CheckEvent(int &kq, struct ::kevent *events, int &event_count);
	void					CloseAllConnection();
	void					CloseVConnection(int sock_client);
	void					CloseConnectionMap(int sock_client);
	void					AddConnectionMap(int, Connection*);
	void					RemoveConnectionMap(int fd);
	void					CheckConnectionTimeout();
	void					AfterProcess(Connection* connection);
	void					Scaffold(Connection* connection);



	void					managerstatus();



private:
	ServerManager(const ServerManager& ref);
	ServerManager& operator=(const ServerManager& ref);
	std::map<int, Connection*>				connectionmap;
	std::vector<Connection*>				v_connection;
	Session									session;
	
	int										kq;
	int										event_count;
	int										sock_serv;
	std::vector<size_t>						v_sock_serv;
	struct sockaddr_in						addr_serv;
};

#endif
