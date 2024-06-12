#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		std::cerr << "Invalid argument\n";
		return 1;
	}

	char buffer[256];

	struct sockaddr_in addr_serv;
	struct sockaddr_in addr_client;
	socklen_t addr_client_len = sizeof(addr_client_len);

	memset(&addr_serv, 0, sizeof(addr_serv));
	addr_serv.sin_family = AF_INET;
	addr_serv.sin_port = htons(atoi(argv[1]));
	addr_serv.sin_addr.s_addr = htonl(INADDR_ANY);

	int sock_serv = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_serv == -1)
	{
		std::cerr << "socket error\n";
		close(sock_serv);
		exit(1);
	}

	if (bind(sock_serv, (struct sockaddr*)&addr_serv, sizeof(addr_serv)) == -1)
	{
		std::cerr << "bind error\n";
		close(sock_serv);
		exit(1);	
	}

	if (listen(sock_serv, 16) == -1)
	{
		std::cerr << "listen error\n";
		close(sock_serv);
		exit(1);
	}

	int sock_client = accept(sock_serv, (sockaddr*)&addr_client, &addr_client_len);
	if (sock_client == -1)
	{
		std::cerr << "accept error\n";
		close(sock_serv);
		exit(1);
	}

	while (1)
	{
		memset(buffer, 0, 256);
		int read_chk = read(sock_client, buffer, sizeof(buffer) - 1);
		if (read_chk == -1)
		{
			std::cerr << "read error\n";
			break ;
		}
		buffer[strlen(buffer)] = '\n';
		std::cout << buffer;
		int write_chk = write(sock_client, buffer, strlen(buffer));
		if (write_chk == -1)
		{
			std::cerr << "write error\n";
			break ;
		}
	}
	close(sock_serv);
	return (0);
}