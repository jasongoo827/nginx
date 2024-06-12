#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		std::cerr << "Invalid argument\n";
		return 1;
	}

	struct sockaddr_in addr_serv;

	memset(&addr_serv, 0, sizeof(addr_serv));
	addr_serv.sin_family = AF_INET;
	addr_serv.sin_addr.s_addr = inet_addr(argv[1]);
	addr_serv.sin_port = htons(atoi(argv[2]));

	int sock_client = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_client == -1)
	{
		std::cerr << "socket error\n";
		close(sock_client);
		exit(1);
	}

	if (connect(sock_client, (struct sockaddr*)&addr_serv, sizeof(addr_serv)) == -1)
	{
		std::cerr << "connect error\n";
		close(sock_client);
		exit(1);
	}

	char readBuf[256];
	char writeBuf[256];

	while (1)
	{
		memset(readBuf, 0, sizeof(readBuf));
		std::cin >> writeBuf;
		if (strlen(writeBuf) > 256)
			break ;
		int write_chk = write(sock_client, writeBuf, strlen(writeBuf));
		if (write_chk == -1)
		{
			std::cerr << "write error\n";
			break ;
		}
		int read_chk = read(sock_client, readBuf, sizeof(readBuf) - 1);
		if (read_chk == -1)
		{
			std::cerr << "read error\n";
			break ;
		}
		readBuf[strlen(readBuf) - 1] = '\n';
		std::cout << readBuf;
	}
	close(sock_client);
	return (0);
}