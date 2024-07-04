#ifndef CONNECTION_HPP
# define CONNECTION_HPP

# include "Response.hpp"
# include "Request.hpp"
# include "Config.hpp"
# include "Server.hpp"
# include "Locate.hpp"
# include "Cgi.hpp"
# include "Enum.hpp"
# include <sys/event.h>
# include <ctime>


class Connection
{
public:
	Connection(int clinet_socket_fd, sockaddr_in client_socket_addr, Config* config_ptr);
	Connection(const Connection& ref);
	~Connection();
	Connection& operator=(const Connection& ref);

	void						MainProcess(struct kevent& event);
	void						ReadClient();
	void						MakeResponse();
	void						ProcessDir();
	void						ProcessFile();
	void						ProcessCgi();
	void						ReadFile();
	void						ReadCgi();
	void						SendCgi();
	void						SendMessage();


	int							GetClientSocketFd();
	int							GetPipein();
	int							GetPipeout();
	enum CurrentProgress		GetProgress();
	int							GetFileFd();

private:
	int						client_socket_fd;
	struct sockaddr_in		client_socket_addr;
	std::string				path;
	Request					request;
	Response				response;
	Cgi						cgi;
	enum CurrentProgress	progress;
	Config*					config_ptr;
	Server const *			server_ptr;
	Locate const *			locate_ptr;
	int						file_fd;
	int						pipein;
	int						pipeout;
	time_t					timeval;
};

#endif
