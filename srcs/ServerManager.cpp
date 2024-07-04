#include "ServerManager.hpp"
#include "Parser.hpp"
#include "Config.hpp"
#include "Server.hpp"
#include "Request.hpp"
#include "Connection.hpp"
#include "Utils.hpp"
// #include "monitor.hpp"
#include <iostream>

ServerManager::ServerManager()
{

}

ServerManager::ServerManager(const ServerManager& ref)
{
	(void)ref;
}

ServerManager::~ServerManager()
{

}

ServerManager& ServerManager::operator=(const ServerManager& ref)
{
	(void)ref;
	return (*this);
}

bool		ServerManager::RunServer(Config* config)
{
	struct kevent				change_event;
	struct kevent				events[20]; /* config에서 파싱 가능하다면 동적 할당 방식으로 변경해야함 */

	while(true)
	{
		/* 서버 어드레스에 대한 정보를 초기화한다. 추후 config 파싱 된 port로 변경 */
		if (InitServerAddress(addr_serv, config->GetServerVec().front().GetPort()) == false)
			continue ;

		/* 서버 소켓을 생성한 뒤 non-blocking 모드로 활성화한다 */
		if (InitServerSocket(sock_serv, addr_serv) == false)
			continue ;

		/* kq를 생성하고, 서버 소켓을 kqueue에 등록 */
		if (InitKqueue(kq, sock_serv, change_event) == false)
			continue ;

		/* 서버 소켓에 연결 요청이 들어온 경우
		새로운 소켓에 대해 accept하고 fd를 리턴값으로 받아 sock_client에 저장한다 */
		std::cout << "socket setting done\n";
		while (1)
		{
			if (CheckEvent(kq, events, event_count, sock_serv) == false)
			{
				std::cout << "no event occured\n";
				break ;
			}
			std::cout << "event occured " << event_count << "\n";
			for (int i = 0; i < event_count; ++i)
			{
				if (events[i].filter == EVFILT_WRITE)
					std::cout << events[i].ident << ", write, " << events[i].flags << "\n\n";
				else if (events[i].filter == EVFILT_READ)
					std::cout << events[i].ident << ", read, " << events[i].flags << "\n\n";
				if (events[i].filter == EVFILT_READ && events[i].ident == static_cast<size_t>(sock_serv))
				{
					int 				sock_client;
					struct sockaddr_in	addr_client;

					std::cout << "sock_serv: " << sock_serv << "\n";
					if (InitClientSocket(kq, sock_serv, change_event, sock_client, addr_client, sizeof(addr_client)) == false)
						continue ; // 실패한 client를 제외한 나머지 이벤트에 대한 처리를 위해 continue
					Connection *con = new Connection(sock_client, addr_client, config);
					v_connection.push_back(con);
					std::cout << "\n----after push_back----\n";
					managerstatus();
					std::cout << v_connection.back()->GetClientSocketFd() << " is add to vec\n";
					AddConnectionMap(sock_client, v_connection.back());
					std::cout << "In socketfd in map: " << connectionmap[sock_client]->GetClientSocketFd()<< "\n";
					std::cout << "client accepted\n";
					std::cout << sock_client << "\n";
				}
				else
				{
					std::cout << "\n----client event----\n";
					managerstatus();

					std::cout << "before MainProcess: events.ident= " << events[i].ident << "\n";
					std::cout << "connectionmap size = " << connectionmap.size() << "\n";
					if (connectionmap.find(static_cast<int>(events[i].ident)) == connectionmap.end())
					{
						std::cout << "cannot find map\n";
						continue;
					}
					Connection* connection = connectionmap[static_cast<int>(events[i].ident)];
					std::cout << "socket_fd: " << connectionmap[static_cast<int>(events[i].ident)]->GetClientSocketFd() << '\n';
					connection->MainProcess(events[i]);
					if (connection->GetProgress() == END_CONNECTION)
					{
						std::cout << "****connection end****\n";
						CloseConnection(connection->GetClientSocketFd());
						break ;
					}
					if (connection->GetProgress() == FROM_FILE)
					{
						AddReadEvent(connection->GetFileFd());
						AddConnectionMap(connection->GetFileFd(), connection);
					}
					else if (connection->GetProgress() == TO_CLIENT)
					{
						if (connection->GetFileFd())
							RemoveReadEvent(connection->GetFileFd());
						AddWriteEvent(connection->GetClientSocketFd());
					}
					// else if (connection->GetProgress() == TO_CGI)
					// {
					// 	AddReadEvent(connection->GetPipein());
					// 	AddWriteEvent(connection->GetPipeout());
					// }
				}
			}
			managerstatus();
		}
		CloseAllConnection();
	}
	return (0);
}

bool	ServerManager::InitServerAddress(sockaddr_in &addr_serv, int port)
{
	void	*error = memset(&addr_serv, 0, sizeof(addr_serv));
	addr_serv.sin_family = AF_INET;
	addr_serv.sin_port = htons(port);
	addr_serv.sin_addr.s_addr = htonl(INADDR_ANY);
	if (error == NULL)
	{
		std::cerr << "memset error\n";
		return (false);
	}
	return (true);
}

bool	ServerManager::InitServerSocket(int &sock_serv, sockaddr_in &addr_serv)
{
	/* 서버가 사용할 소켓 생성 */
	sock_serv = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_serv == -1)
	{
		std::cerr << "socket error\n";
		return (false);
	}
	/* 서버가 사용하는 소켓을 재활용 가능하도록 설정 */
	int enable = 1;
	if (setsockopt(sock_serv, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
	{
		std::cerr << "setsockopt fail\n";
		close(sock_serv);
		return (false);
	}
	/* 서버가 사용할 소켓에 서버의 정보 등록 */
	if (bind(sock_serv, (struct sockaddr*)&addr_serv, sizeof(addr_serv)) == -1)
	{
		std::cerr << "bind error\n";
		close(sock_serv);
		return (false);
	}
	/* 서버 소켓에 대한 통신 활성화 */
	if (listen(sock_serv, 16) == -1)
	{
		std::cerr << "listen error\n";
		close(sock_serv);
		return (false);
	}
	/* NonBlocking 설정 */
	if (utils::SetNonBlock(sock_serv) == false)
	{
		std::cerr << "sock_serv nonblock error\n";
		close(sock_serv);
		return (false);
	}
	return (true);
}

bool	ServerManager::InitClientSocket(int &kq, int &sock_serv, struct ::kevent &change_event, int &sock_client, sockaddr_in &addr_client, socklen_t addr_client_len)
{
	std::cout << "start init socket\n";
	/* 서버 소켓으로 요청이 발생하는 경우 연결 요청이므로 신규 클라이언트 연결 */
	sock_client = accept(sock_serv, (sockaddr*)&addr_client, &addr_client_len);
	if (sock_client < 0)
	{
		if (errno == EWOULDBLOCK || errno == EAGAIN) {
			std::cout << "Connection not found\n";
			return (false);
		}
		else
		{
			std::cerr << "accept fail\n";
			return (false);
		}
	}
	/* 소켓을 재활용 가능하도록 설정 */
	int enable = 1;
	if (setsockopt(sock_client, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
	{
		std::cerr << "setsockopt fail\n";
		close(sock_client);
		return (false);
	}
	/* NonBlocking 설정 */
	if (utils::SetNonBlock(sock_client) == false)
	{
		std::cerr << "sock_client nonblock error\n";
		close(sock_client);
		return (false);
	}
	EV_SET(&change_event, sock_client, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	if (::kevent(kq, &change_event, 1, NULL, 0, NULL) == -1) {
		std::cerr << "sock_serv kevent registration fail\n";
		close(sock_client);
		return (false);
	}
	std::cout << "\n----after init client_socket----\n";
	managerstatus();
	return (true);
}

bool	ServerManager::InitKqueue(int &kq, int &sock_serv, struct ::kevent &change_event)
{
	kq = kqueue();
	if (kq == -1)
	{
		std::cerr << "kqueue fail\n";
		close(sock_serv);
		return (false);
	}

	EV_SET(&change_event, sock_serv, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	if (::kevent(kq, &change_event, 1, NULL, 0, NULL) == -1)
	{
		std::cerr << "sock_serv kevent registration fail\n";
		close(sock_serv);
		close(kq);
		return (false);
	}
	return (true);
}

bool	ServerManager::CheckEvent(int &kq, struct ::kevent *events, int &event_count, int &sock_serv)
{
	event_count = kevent(kq, NULL, 0, events, 20, NULL); /* config에서 파싱 가능하다면 20 대신 config 설정 수치로 변경해야함 */
	if (event_count == -1)
	{
		std::cerr << "kevent wait fail\n";
		close(sock_serv);
		close(kq);
		return (false);
	}
	return (true);
}

void	ServerManager::CloseConnection(int sock_client)
{
	struct ::kevent change_event;

	std::cout << "Close connection for fd: " << sock_client << "\n";
	EV_SET(&change_event, sock_client, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	kevent(kq, &change_event, 1, NULL, 0, NULL);
	std::cout << "remove read event\n";
	std::vector<Connection>::iterator it;
	std::cout <<"v_connection size: "<< v_connection.size() << "\n";

	for (size_t i = 0; i < v_connection.size(); i++)
	{
		if (v_connection[i]->GetClientSocketFd() == sock_client)
		{
			RemoveConnectionMap(v_connection[i]->GetClientSocketFd());
			std::cout << "filefd connected: " << i << v_connection[i]->GetFileFd() << "\n";
			if (v_connection[i]->GetFileFd())
			{
				RemoveConnectionMap(v_connection[i]->GetFileFd());
				close(v_connection[i]->GetFileFd());
			}
			// if (it->GetCgiFd())
			// {
			// 	RemoveConnectionMap(it->GetCgiFd);
			// }
			delete v_connection[i];
			v_connection.erase(v_connection.begin() + i);
		}
	}
	close(sock_client);
	std::cout << "close connection " << sock_client << "\n";
}

void	ServerManager::CloseAllConnection()
{
	for (size_t i = 0; i < v_connection.size(); ++i)
		close(v_connection[i]->GetClientSocketFd());
	close(sock_serv);
	close(kq);
	v_connection.clear();
	connectionmap.clear();
}

void		ServerManager::AddConnectionMap(int fd, Connection* connection)
{
	std::cout << "connectionmap size: " << connectionmap.size() << "\n";
	connectionmap[fd] = connection;
	std::cout << "map added\n";
	std::cout << "connectionmap size: " << connectionmap.size() << "\n";
}

void		ServerManager::RemoveConnectionMap(int fd)
{
	size_t erasesize = connectionmap.erase(fd);
	std::cout << "map erased "<< erasesize << "times for fd" << fd << ", map size: "<< connectionmap.size() << "\n";
}

void		ServerManager::AddWriteEvent(int client_socket_fd)
{
	struct kevent change_event;
	EV_SET(&change_event, client_socket_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
	std::cout << "kq: " << kq << "\n";
	int ret = kevent(kq, &change_event, 1, NULL, 0, NULL);
	if (ret < 0)
	{
		std::cout << "fail to add writeevent\n";
		std::cout << errno << "\n";
	}
}

void		ServerManager::RemoveWriteEvent(int client_socket_fd)
{
	struct kevent change_event;
	EV_SET(&change_event, client_socket_fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
	std::cout << "try to remove event: fd: " << client_socket_fd << "\n";
	int ret = kevent(kq, &change_event, 1, NULL, 0, NULL);
	if (ret < 0)
	{
		std::cout << "fail to remove writeevent\n";
		std::cout << errno << "\n";
	}
}

void		ServerManager::AddReadEvent(int fd)
{
	struct kevent change_event;
	EV_SET(&change_event, fd, EVFILT_READ, EV_ENABLE | EV_ADD, 0, 0, NULL);
	std::cout << "kq: " << kq << "\n";
	int ret = kevent(kq, &change_event, 1, NULL, 0, NULL);
	if (ret < 0)
	{
		std::cout << "fail to add read event\n";
		std::cout << errno << "\n";
	}
}

void		ServerManager::RemoveReadEvent(int fd)
{
	struct kevent change_event;
	EV_SET(&change_event, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	std::cout << "kq: " << kq << "\n";
	int ret = kevent(kq, &change_event, 1, NULL, 0, NULL);
	if (ret < 0)
	{
		std::cout << "fail to remove read event\n";
		std::cout << errno << "\n";
	}
}

void	ServerManager::managerstatus()
{
	std::cout << "------------------------------------------\n";
	std::cout << "vconnection size = " << v_connection.size() << "\n";
	for (size_t i = 0; i < v_connection.size(); i++)
		std::cout << "connection" << i << ": " << "clientsocket: " << v_connection[i]->GetClientSocketFd() << "\n";
	std::cout << "connectionmap size = " << connectionmap.size() << "\n";
	for (std::map<int, Connection*>::iterator it = connectionmap.begin(); it != connectionmap.end(); it++)
		std::cout << "connectionmap: "<< it->first << " clientsocket: " << it->second->GetClientSocketFd() <<  "\n";
	std::cout << "------------------------------------------\n";
}
