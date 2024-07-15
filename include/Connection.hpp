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
# include "Session.hpp"
# include "Parser.hpp"

class Session;
class Parser;

class Connection
{
public:
	Connection(int kq, int clinet_socket_fd, sockaddr_in client_socket_addr, Config* config_ptr, Session* session);
	Connection(const Connection& ref);
	~Connection();
	Connection& operator=(const Connection& ref);

	void						MainProcess(struct kevent& event);
	void						ReadClient();
	void						MakeResponse();
	void						ProcessGet();
	void						ProcessPost();
	void						ProcessDelete();
	void						ProcessDir();
	void						ProcessFile();
	void						ProcessCgi();
	void						ReadFile();
	void						ReadCgi();
	void						SendCgi();
	void						SendMessage();
	void						SetPipein(int fd);
	void						SetPipeout(int fd);
	void						SetFileFd(int fd);

	int							GetClientSocketFd();
	int							GetPipein();
	int							GetPipeout();
	enum CurrentProgress		GetProgress();
	int							GetFileFd();
	std::time_t					GetTimeval();
	Parser&						GetParser();
	Request&					GetRequest();
	Response&					GetResponse();
	Cgi&						GetCgi();
	int							GetKq();
	void						SetProgress(enum CurrentProgress progress);
	void						UpdateTimeval();
	void						Cleaner();
	void						CheckExitCgi();


private:
	int						kq;
	int						client_socket_fd;
	struct sockaddr_in		client_socket_addr;
	std::string				path;
	Parser					parser;
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
	std::time_t				timeval;
	Session					*session;
	int						total_len;
};

#endif
