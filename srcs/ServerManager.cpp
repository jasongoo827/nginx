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
	struct kevent				events[60];

	while(true)
	{
		if (config->GetServerVec().empty())
		{
			std::cerr << "No server configuration\n";
			return false;
		}
		if (InitKqueue(kq, sock_serv) == false)
			continue ;
		for (std::vector<Server>::const_iterator it = config->GetServerVec().begin(); it != config->GetServerVec().end(); ++it)
		{
			if (InitServerAddress(kq, addr_serv, it->GetPort()) == false)
			{
				CloseAllServsock();
				break ;
			}
			if (InitServerSocket(kq, sock_serv, addr_serv) == false)
			{
				CloseAllServsock();
				break ;
			}
			if (RegistSockserv(kq, sock_serv, change_event) == false)
			{
				CloseAllServsock();
				break ;
			}
			v_sock_serv.push_back(sock_serv);
		}
		if (kq == 0)
			continue ;
		while (1)
		{
			if (CheckEvent(kq, events, event_count) == false)
				break ;
			for (int i = 0; i < event_count; ++i)
			{
				Connection* connection = static_cast<Connection*>(events[i].udata);

				// 새로운 클라이언트 연결 (서버 소켓 이벤트)
				if (connection == NULL && events[i].filter == EVFILT_READ && CheckValidServer(events[i].ident))
				{
					int 				sock_client;
					struct sockaddr_in	addr_client;
					if (InitClientSocket(kq, sock_serv, change_event, sock_client, addr_client, sizeof(addr_client), NULL) == false)
						continue ;
					Connection *con = new Connection(kq, sock_client, addr_client, config, &session);
					v_connection.push_back(con);
					// 이벤트에 Connection 포인터 재등록
					struct kevent update_event;
					EV_SET(&update_event, sock_client, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, con);
					kevent(kq, &update_event, 1, NULL, 0, NULL);
				}
				// 정상 이벤트 처리
				else if (connection != NULL && (!(events[i].flags & EV_EOF) || (events[i].filter == EVFILT_READ && connection->GetProgress() == CGI)))
				{
					connection->MainProcess(events[i]);
					connection->UpdateTimeval();
					AfterProcess(connection);
				}
				// 연결 종료 처리 (EV_EOF)
				else if (connection != NULL && (events[i].flags & EV_EOF))
				{
					HandleConnectionEOF(connection, events[i].ident);
				}
			}
			CheckConnectionTimeout();
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
	sock_serv = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_serv == -1)
	{
		std::cerr << "socket error\n";
		close(kq);
		kq = 0;
		return (false);
	}
	int enable = 1;
	if (setsockopt(sock_serv, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
	{
		std::cerr << "setsockopt fail\n";
		close(sock_serv);
		close(kq);
		kq = 0;
		return (false);
	}
	if (bind(sock_serv, (struct sockaddr*)&addr_serv, sizeof(addr_serv)) == -1)
	{
		std::cerr << "bind error\n";
		close(sock_serv);
		close(kq);
		kq = 0;
		return (false);
	}
	if (listen(sock_serv, 2048) == -1)
	{
		std::cerr << "listen error\n";
		close(sock_serv);
		close(kq);
		kq = 0;
		return (false);
	}
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

bool	ServerManager::InitClientSocket(int &kq, int &sock_serv, struct ::kevent &change_event, int &sock_client, sockaddr_in &addr_client, socklen_t addr_client_len, void* udata)
{
	sock_client = accept(sock_serv, (sockaddr*)&addr_client, &addr_client_len);
	if (sock_client < 0)
	{
		std::cerr << "accept fail\n";
		return (false);
	}
	int enable = 1;
	if (setsockopt(sock_client, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0)
	{
		std::cerr << "setsockopt fail\n";
		close(sock_client);
		return (false);
	}
	if (utils::SetNonBlock(sock_client) == false)
	{
		std::cerr << "sock_client nonblock error\n";
		close(sock_client);
		return (false);
	}
	EV_SET(&change_event, sock_client, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, udata);
	if (::kevent(kq, &change_event, 1, NULL, 0, NULL) == -1) {
		std::cerr << "sock_serv kevent registration fail\n";
		close(sock_client);
		return (false);
	}
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
	event_count = kevent(kq, NULL, 0, events, 60, NULL);
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

	EV_SET(&change_event, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	kevent(kq, &change_event, 1, NULL, 0, NULL);
	EV_SET(&change_event, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
	kevent(kq, &change_event, 1, NULL, 0, NULL);
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
}

void	ServerManager::RemoveEventAndClose(int fd)
{
	struct ::kevent change_event;

	EV_SET(&change_event, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	kevent(kq, &change_event, 1, NULL, 0, NULL);
	EV_SET(&change_event, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
	kevent(kq, &change_event, 1, NULL, 0, NULL);
	close(fd);
}

void	ServerManager::CloseAllConnection()
{
	for (size_t i = 0; i < v_connection.size(); ++i)
	{
		if (v_connection[i]->GetFileFd())
		{
			RemoveEventAndClose(v_connection[i]->GetFileFd());
			v_connection[i]->SetFileFd(0);
		}
		else if (v_connection[i]->GetPipein())
		{
			RemoveEventAndClose(v_connection[i]->GetPipein());
			v_connection[i]->SetPipein(0);
		}
		else if (v_connection[i]->GetPipeout())
		{
			RemoveEventAndClose(v_connection[i]->GetPipeout());
			v_connection[i]->SetPipeout(0);
		}
		CloseVConnection(v_connection[i]->GetClientSocketFd());
	}
	close(kq);
	v_connection.clear();
}

void	ServerManager::managerstatus()
{
	std::cout << "------------------------------------------\n";
	std::cout << "vconnection size = " << v_connection.size() << "\n";
	for (size_t i = 0; i < v_connection.size(); i++)
		std::cout << "connection" << i << ": " << "clientsocket: " << v_connection[i]->GetClientSocketFd() << "\n";
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
				RemoveEventAndClose(v_connection[i]->GetFileFd());
				v_connection[i]->SetFileFd(0);
			}
			else if (v_connection[i]->GetPipein())
			{
				RemoveEventAndClose(v_connection[i]->GetPipein());
				v_connection[i]->SetPipein(0);
			}
			else if (v_connection[i]->GetPipeout())
			{
				RemoveEventAndClose(v_connection[i]->GetPipeout());
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
		if (connection->GetRequest().FindValueInHeader("connection") == "close")
			CloseVConnection(connection->GetClientSocketFd());
		else
		{
			utils::RemoveWriteEvent(kq, connection->GetClientSocketFd());
			utils::AddReadEvent(kq, connection->GetClientSocketFd(), connection);
			Scaffold(connection);
		}
	}
	else if (connection->GetProgress() == FROM_FILE)
	{
		utils::AddReadEventForFile(kq, connection->GetFileFd(), connection);
	}
	else if (connection->GetProgress() == CGI)
	{
		// udata에 이미 connection 포인터가 설정됨 (Connection.cpp에서 처리)
	}
	else if (connection->GetProgress() == COMBINE)
	{
		if (connection->GetPipeout())
		{
			utils::RemoveWriteEvent(kq, connection->GetPipeout());
			RemoveEventAndClose(connection->GetPipeout());
			connection->SetPipeout(0);
		}
		if (connection->GetFileFd())
		{
			RemoveEventAndClose(connection->GetFileFd());
			connection->SetFileFd(0);
		}
		if (connection->GetPipein())
		{
			RemoveEventAndClose(connection->GetPipein());
			connection->SetPipein(0);
		}
		connection->CheckExitCgi();
		utils::AddWriteEvent(kq, connection->GetClientSocketFd(), connection);
		utils::RemoveReadEvent(kq, connection->GetClientSocketFd());
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
	utils::AddReadEvent(kq, connection->GetClientSocketFd(), connection);
}

void	ServerManager::HandleConnectionEOF(Connection* connection, int fd)
{
	// v_connection에서 해당 connection 찾기
	bool found_in_vector = false;
	for (size_t idx = 0; idx < v_connection.size(); idx++)
	{
		if (v_connection[idx] == connection)
		{
			found_in_vector = true;
			break;
		}
	}

	// v_connection에 없으면 단순히 fd만 닫기
	if (!found_in_vector)
	{
		RemoveEventAndClose(fd);
		return;
	}

	// v_connection에 있으면 해당 Connection의 다른 fd 개수 확인
	int count = 0;
	if (connection->GetClientSocketFd() == fd)
		count++;
	if (connection->GetFileFd() && connection->GetFileFd() == fd)
		count++;
	if (connection->GetPipein() && connection->GetPipein() == fd)
		count++;
	if (connection->GetPipeout() && connection->GetPipeout() == fd)
		count++;

	// fd 닫기 (공통)
	RemoveEventAndClose(fd);

	// 해당 fd가 이 connection의 유일한 fd인 경우 Connection도 정리
	if (count == 1)
		CloseVConnection(connection->GetClientSocketFd());
}