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
	kq = 0;
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
	struct kevent				events[60]; /* config에서 파싱 가능하다면 동적 할당 방식으로 변경해야함 */

	while(true)
	{
		if (config->GetServerVec().empty())
		{
			std::cerr << "No server configuration\n";
			return false;
		}

		/* kq를 생성 */
		if (InitKqueue(kq, sock_serv) == false)
			continue ;

		for (std::vector<Server>::const_iterator it = config->GetServerVec().begin(); it != config->GetServerVec().end(); ++it)
		{
			/* 서버 어드레스에 대한 정보를 초기화한다. 추후 config 파싱 된 port로 변경 */
			if (InitServerAddress(kq, addr_serv, it->GetPort()) == false)
			{
				CloseAllServsock();
				break ;
			}

			/* 서버 소켓을 생성한 뒤 non-blocking 모드로 활성화한다 */
			if (InitServerSocket(kq, sock_serv, addr_serv) == false)
			{
				CloseAllServsock();
				break ;
			}

			/* 서버 소켓을 kqueue에 등록 */
			if (RegistSockserv(kq, sock_serv, change_event) == false)
			{
				CloseAllServsock();
				break ;
			}
			v_sock_serv.push_back(sock_serv);
			std::cout << "added sock_serv: " << v_sock_serv.back() << '\n';
		}
		std::cout << "v_sock_serv size: " << v_sock_serv.size() << '\n';
		if (kq == 0)
			continue ;

		/* 서버 소켓에 연결 요청이 들어온 경우
		새로운 소켓에 대해 accept하고 fd를 리턴값으로 받아 sock_client에 저장한다 */
		std::cout << "socket setting done\n";
		while (1)
		{
			if (CheckEvent(kq, events, event_count) == false)
				break ;
			std::cout << "event occured " << event_count << "\n";
			for (int i = 0; i < event_count; ++i)
			{
				if (events[i].filter == EVFILT_WRITE)
					std::cout << events[i].ident << ", write, " << events[i].flags << "\n\n";
				else if (events[i].filter == EVFILT_READ)
					std::cout << events[i].ident << ", read, " << events[i].flags << "\n\n";
				if (events[i].filter == EVFILT_READ && CheckValidServer(events[i].ident))
				{
					int 				sock_client;
					struct sockaddr_in	addr_client;
					std::cout << "sock_serv: " << sock_serv << "\n";
					if (InitClientSocket(kq, sock_serv, change_event, sock_client, addr_client, sizeof(addr_client)) == false)
						continue ; // 실패한 client를 제외한 나머지 이벤트에 대한 처리를 위해 continue
					Connection *con = new Connection(kq, sock_client, addr_client, config, &session);
					v_connection.push_back(con);
					std::cout << "\n----after push_back----\n";
					// managerstatus();
					std::cout << v_connection.back()->GetClientSocketFd() << " is add to vec\n";
					AddConnectionMap(sock_client, v_connection.back());
					std::cout << "In socketfd in map: " << connectionmap[sock_client]->GetClientSocketFd()<< "\n";
					std::cout << "client accepted\n";
					std::cout << sock_client << "\n";
				}
				else if (!(events[i].flags & EV_EOF) || (connectionmap.find(static_cast<int>(events[i].ident)) != connectionmap.end() && (events[i].filter == EVFILT_READ && connectionmap[static_cast<int>(events[i].ident)]->GetProgress() == CGI)))
				{
					std::cout << "\n----client event----\n";
					// managerstatus();
					std::cout << "before MainProcess: events.ident= " << events[i].ident << "\n";
					std::cout << "connectionmap size = " << connectionmap.size() << "\n";
					if (connectionmap.find(static_cast<int>(events[i].ident)) == connectionmap.end())
					{
						std::cout << "cannot find map\n";
						continue;
					}
					Connection* connection = connectionmap[static_cast<int>(events[i].ident)];
					std::cout << "socket_fd: " << connection->GetClientSocketFd() << '\n';
					connection->MainProcess(events[i]);
					// progress = cgi
					connection->UpdateTimeval();
					AfterProcess(connection);
				}
				else if (events[i].flags & EV_EOF)
				{
					size_t	idx = 0;
					for (idx = 0; idx < v_connection.size(); idx++)
					{
						if (v_connection[idx]->GetClientSocketFd() == static_cast<int>(events[i].ident))
							break ;
					}
					if (idx == v_connection.size())
						CloseConnectionMap(events[i].ident);
					else
					{
						int	count = 0;
						for (std::map<int, Connection*>::iterator it = connectionmap.begin(); it != connectionmap.end(); ++it)
						{
							if (it->second->GetClientSocketFd() == static_cast<int>(events[i].ident))
								count++;
						}
						if (count == 1)
						{
							CloseConnectionMap(events[i].ident);
							CloseVConnection(events[i].ident);
						}
					}
				}
			}
			// managerstatus();
			std::cout << "fd usage: " << connectionmap.size() << '\n';
			std::cout << "client units: " << v_connection.size() << '\n';
			CheckConnectionTimeout();
			std::cout << "--------------------cycle ended----------------------------\n";
		}
		CloseAllConnection();
	}
	return (0);
}

bool	ServerManager::CheckValidServer(size_t &sock)
{
	for (std::vector<size_t>::iterator it = v_sock_serv.begin(); it != v_sock_serv.end(); ++it)
	{
		if (sock == *it)
		{
			sock_serv = sock;
			return (true);
		}
	}
	return (false);
}

bool	ServerManager::InitServerAddress(int &kq, sockaddr_in &addr_serv, int port)
{
	void	*error = memset(&addr_serv, 0, sizeof(addr_serv));
	addr_serv.sin_family = AF_INET;
	addr_serv.sin_port = htons(port);
	addr_serv.sin_addr.s_addr = htonl(INADDR_ANY);
	if (error == NULL)
	{
		std::cerr << "memset error\n";
		close(kq);
		kq = 0;
		return (false);
	}
	return (true);
}

bool	ServerManager::InitServerSocket(int &kq, int &sock_serv, sockaddr_in &addr_serv)
{
	/* 서버가 사용할 소켓 생성 */
	sock_serv = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_serv == -1)
	{
		std::cerr << "socket error\n";
		close(kq);
		kq = 0;
		return (false);
	}
	/* 서버가 사용하는 소켓을 재활용 가능하도록 설정 */
	int enable = 1;
	if (setsockopt(sock_serv, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
	{
		std::cerr << "setsockopt fail\n";
		close(sock_serv);
		close(kq);
		kq = 0;
		return (false);
	}
	/* 서버가 사용할 소켓에 서버의 정보 등록 */
	std::cout << "port: " << addr_serv.sin_port << '\n';
	if (bind(sock_serv, (struct sockaddr*)&addr_serv, sizeof(addr_serv)) == -1)
	{
		std::cerr << "bind error\n";
		close(sock_serv);
		close(kq);
		kq = 0;
		return (false);
	}
	/* 서버 소켓에 대한 통신 활성화 */
	if (listen(sock_serv, 2048) == -1)
	{
		std::cerr << "listen error\n";
		close(sock_serv);
		close(kq);
		kq = 0;
		return (false);
	}
	/* NonBlocking 설정 */
	if (utils::SetNonBlock(sock_serv) == false)
	{
		std::cerr << "sock_serv nonblock error\n";
		close(sock_serv);
		close(kq);
		kq = 0;
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
		std::cerr << "accept fail\n";
		return (false);
	}
	/* 소켓을 재활용 가능하도록 설정 */
	int enable = 1;
	if (setsockopt(sock_client, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0)
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
	// managerstatus();
	return (true);
}

bool	ServerManager::InitKqueue(int &kq, int &sock_serv)
{
	kq = kqueue();
	if (kq == -1)
	{
		std::cerr << "kqueue fail\n";
		close(sock_serv);
		return (false);
	}
	return (true);
}

bool	ServerManager::RegistSockserv(int &kq, int &sock_serv, struct ::kevent &change_event)
{
	EV_SET(&change_event, sock_serv, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	if (::kevent(kq, &change_event, 1, NULL, 0, NULL) == -1)
	{
		std::cerr << "sock_serv kevent registration fail\n";
		close(sock_serv);
		close(kq);
		kq = 0;
		return (false);
	}
	return (true);
}

bool	ServerManager::CheckEvent(int &kq, struct ::kevent *events, int &event_count)
{
	event_count = kevent(kq, NULL, 0, events, 20, NULL); /* config에서 파싱 가능하다면 20 대신 config 설정 수치로 변경해야함 */
	if (event_count == -1)
	{
		std::cerr << "kevent wait fail\n";
		return (false);
	}
	return (true);
}

void	ServerManager::CloseAllServsock()
{
	if (v_sock_serv.empty())
		return ;
	while (!v_sock_serv.empty())
	{
		close(v_sock_serv.back());
		v_sock_serv.pop_back();
	}
}

void	ServerManager::CloseVConnection(int fd)
{
	struct ::kevent change_event;

	std::cout << "Close connection for fd: " << fd << "\n";
	EV_SET(&change_event, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	kevent(kq, &change_event, 1, NULL, 0, NULL);
	EV_SET(&change_event, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
	kevent(kq, &change_event, 1, NULL, 0, NULL);
	RemoveConnectionMap(fd);
	for (size_t i = 0; i < v_connection.size(); i++)
	{
		if (v_connection[i]->GetClientSocketFd() == fd)
		{
			v_connection[i]->GetCgi().Cleaner();
			delete v_connection[i];
			v_connection.erase(v_connection.begin() + i);
		}
	}
	close(fd);
	std::cout << "close connection " << fd << "\n";
}

void	ServerManager::CloseConnectionMap(int fd)
{
	struct ::kevent change_event;

	// std::cout << "Close connection for fd: " << fd << ", connection : " << connectionmap[fd]->GetClientSocketFd() << "\n";
	EV_SET(&change_event, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	kevent(kq, &change_event, 1, NULL, 0, NULL);
	EV_SET(&change_event, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
	kevent(kq, &change_event, 1, NULL, 0, NULL);
	RemoveConnectionMap(fd);
	close(fd);
	std::cout << "close connectionmap " << fd << "\n";
}

void	ServerManager::CloseAllConnection()
{
	for (size_t i = 0; i < v_connection.size(); ++i)
	{
		if (v_connection[i]->GetFileFd())
		{
			CloseConnectionMap(v_connection[i]->GetFileFd());
			v_connection[i]->SetFileFd(0);
		}
		else if (v_connection[i]->GetPipein())
		{
			CloseConnectionMap(v_connection[i]->GetPipein());
			v_connection[i]->SetPipein(0);
		}
		else if (v_connection[i]->GetPipeout())
		{
			CloseConnectionMap(v_connection[i]->GetPipeout());
			v_connection[i]->SetPipeout(0);
		}
		CloseVConnection(v_connection[i]->GetClientSocketFd());
	}
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

void	ServerManager::CheckConnectionTimeout()
{
	std::time_t now;
	std::time(&now);
	for(size_t i = 0; i < v_connection.size(); i++)
	{
		if (std::difftime(now, v_connection[i]->GetTimeval()) > 30)
		{
			if (v_connection[i]->GetFileFd())
			{
				CloseConnectionMap(v_connection[i]->GetFileFd());
				v_connection[i]->SetFileFd(0);
			}
			else if (v_connection[i]->GetPipein())
			{
				CloseConnectionMap(v_connection[i]->GetPipein());
				v_connection[i]->SetPipein(0);
			}
			else if (v_connection[i]->GetPipeout())
			{
				CloseConnectionMap(v_connection[i]->GetPipeout());
				v_connection[i]->SetPipeout(0);
			}
			CloseVConnection(v_connection[i]->GetClientSocketFd());
		}
	}
}

void	ServerManager::AfterProcess(Connection* connection)
{
	if (connection->GetProgress() == END_CONNECTION)
	{
		std::cout << "****connection end****\n";
		if (connection->GetRequest().FindValueInHeader("connection") == "close")
			CloseVConnection(connection->GetClientSocketFd());
		else
		{
			utils::RemoveWriteEvent(kq, connection->GetClientSocketFd());
			std::cout << "RemoveWriteEvent1 fd: " << connection->GetClientSocketFd() << '\n';
			utils::AddReadEvent(kq, connection->GetClientSocketFd());
			std::cout << "AddReadEvent1 fd: " << connection->GetClientSocketFd() << '\n';
			Scaffold(connection);
		}
	}
	else if (connection->GetProgress() == FROM_FILE)
	{
		utils::AddReadEventForFile(kq, connection->GetFileFd());
		std::cout << "AddReadEvent3 fd: " << connection->GetFileFd() << '\n';
		AddConnectionMap(connection->GetFileFd(), connection);
	}
	else if (connection->GetProgress() == TO_CLIENT)
	{
		// utils::AddWriteEvent(kq, connection->GetClientSocketFd());
		// std::cout << "AddWriteEvent3 fd: " << connection->GetClientSocketFd() << '\n';
	}
	else if (connection->GetProgress() == CGI)
	{
		AddConnectionMap(connection->GetPipeout(), connection);
		AddConnectionMap(connection->GetPipein(), connection);
	}
	else if (connection->GetProgress() == COMBINE)
	{
		if (connection->GetPipeout())
		{
			utils::RemoveWriteEvent(kq, connection->GetPipeout());
			CloseConnectionMap(connection->GetPipeout());
			std::cout << "RemoveWriteEvent2 fd: " << connection->GetPipeout() << '\n';
			connection->SetPipeout(0);
		}
		if (connection->GetFileFd())
		{
			CloseConnectionMap(connection->GetFileFd());
			connection->SetFileFd(0);
		}
		if (connection->GetPipein())
		{
			CloseConnectionMap(connection->GetPipein());
			connection->SetPipein(0);
		}
		connection->CheckExitCgi();
		utils::AddWriteEvent(kq, connection->GetClientSocketFd());
		std::cout << "AddWriteEvent2 fd: " << connection->GetClientSocketFd() << '\n';
		utils::RemoveReadEvent(kq, connection->GetClientSocketFd());
		std::cout << "RemoveReadEvent3 fd: " << connection->GetClientSocketFd() << '\n';
		connection->GetResponse().CombineMessage();
		connection->SetProgress(TO_CLIENT);
	}
}

void	ServerManager::Scaffold(Connection* connection)
{
	connection->GetCgi().Cleaner();
	connection->GetParser().Cleaner();
	connection->GetRequest().Cleaner();
	connection->GetResponse().Cleaner();
	connection->Cleaner();
	utils::RemoveReadEvent(kq, connection->GetClientSocketFd());
	std::cout << "RemoveReadEvent2 fd: " << connection->GetClientSocketFd() << '\n';
	utils::AddReadEvent(kq, connection->GetClientSocketFd());
	std::cout << "AddReadEvent2 fd: " << connection->GetClientSocketFd() << '\n';
}