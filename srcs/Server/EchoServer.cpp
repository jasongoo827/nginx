#include "Parser.hpp"
#include "Request.hpp"
#include "Config.hpp"
#include "Status.hpp"

// 3629 - utf8 encoding
// 3875 - cgi
// 3986 - uri
// 5234, 7405 - BNF
// 8174 - Upper, Lower
// 9110, 9111, 9112 - HTTP


int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		std::cerr << "Invalid argument\n";
		return (1);
	}
	std::string file(argv[1]);
	Config config;
	Status status = config.ReadConfig(file);
	if (!status.ok())
	{
		std::cerr << status.message() << '\n';
		return (1);
	}
	config.PrintConfigInfo();


	int							kq, event_count;
	struct kevent				change_event;
	struct kevent				events[20]; /* config에서 파싱 가능하다면 동적 할당 방식으로 변경해야함 */
	int							sock_serv;
	struct sockaddr_in			addr_serv;
	std::vector<int>			v_sock_client;
	std::vector<sockaddr_in>	v_addr_client;

	while(true)
	{
		/* 서버 어드레스에 대한 정보를 초기화한다. 추후 config 파싱 된 port로 변경 */
		if (utils::InitServerAddress(addr_serv, argv[1]) == false)
			continue ;

		/* 서버 소켓을 생성한 뒤 non-blocking 모드로 활성화한다 */
		if (utils::InitServerSocket(sock_serv, addr_serv) == false)
			continue ;

		/* kq를 생성하고, 서버 소켓을 kqueue에 등록 */
		if (utils::InitKqueue(kq, sock_serv, change_event) == false)
			continue ;

		/* 서버 소켓에 연결 요청이 들어온 경우
		새로운 소켓에 대해 accept하고 fd를 리턴값으로 받아 sock_client에 저장한다 */

		while (1)
		{
			if (utils::CheckEvent(kq, events, event_count, sock_serv) == false)
			{
				utils::CloseAllConnection(v_sock_client, v_addr_client, kq, sock_serv);
				break ;
			}
			for (int i = 0; i < event_count; ++i)
			{
				if (events[i].filter == EVFILT_READ)
				{
					if (events[i].ident == sock_serv)
					{
						int 				sock_client;
						struct sockaddr_in	addr_client;

						if (utils::InitClientSocket(kq, sock_serv, change_event, sock_client, addr_client, sizeof(addr_client)) == false)
							continue ; // 실패한 client를 제외한 나머지 이벤트에 대한 처리를 위해 continue
						v_sock_client.push_back(sock_client);
						v_addr_client.push_back(addr_client);
					}
					else
					{
						int 				sock_client = events[i].ident;
						struct sockaddr_in	addr_client = v_addr_client[i];
						char				buffer[1000000];
						ssize_t				nread = read(sock_client, buffer, sizeof(buffer));

						if (nread <= 0)
							utils::CloseConnection(change_event, v_sock_client, v_addr_client, kq, sock_client, addr_client);
						else
						{
							std::cout << "\n\n원본 메시지\n" << std::string(buffer, nread) << "\n\n\n";
							Parser	pars_buf(std::string(buffer, nread));
							Request req_1;
							pars_buf.ParseStartline(req_1);
							pars_buf.ParseHeader(req_1);
							pars_buf.ParseBody(req_1);
							pars_buf.ParseTrailer(req_1);
							std::cout << "파싱 메시지\n";
							std::cout << req_1.GetMethod() << " " << req_1.GetUrl() << " " << req_1.GetVersion() << '\n';
							std::map<std::string, std::string> tmp_map = req_1.GetHeader();
							for (std::map<std::string, std::string>::iterator it = tmp_map.begin(); it != tmp_map.end(); ++it)
								std::cout << it->first << ": \'" << it->second << "\'\n";
							std::cout << '\'' << req_1.GetBody() << "\'\n";
							std::cout << "status = " << req_1.GetStatus() << '\n';
						}
					}
				}
				else if (events[i].filter == EVFILT_WRITE)
				{
					int 				sock_client = events[i].ident;
					struct sockaddr_in	addr_client = v_addr_client[i];
					char tmp[256] = "\r\n\r\n<h1>My home page.</h1>\r\n\r\n\0";
					int write_chk = write(sock_client, tmp, strlen(tmp));
					if (write_chk == -1)
					{
						std::cerr << "write error\n";
						break ;
					}
				}
			}
		}
		utils::CloseAllConnection(v_sock_client, v_addr_client, kq, sock_serv);
	}
	return (0);
}