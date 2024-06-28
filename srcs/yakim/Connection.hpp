#ifndef CONNECTION_HPP
# define CONNECTION_HPP

# include "Response.hpp"
# include "Request.hpp"
# include "Config.hpp"
# include "Server.hpp"
# include "Locate.hpp"
# include "Cgi.hpp"
# include "Enum.hpp"

class Connection
{
public:
	Connection();
	Connection(int clinet_socket_fd, Config* config_ptr);
	Connection(const Connection& ref);
	~Connection();
	Connection& operator=(const Connection& ref);

	void	mainprocess();
	void	makeResponse();
	void	parserequest();
	void	processDir();
	void	processFile();
	void	processCgi();
	void	readFile();
	void	readCgi();
	void	sendCgi();
	void	sendMessage();

private:
	int						client_socket_fd;
	std::string				path;
	Request					request;
	Response				response;
	Cgi						cgi;
	enum CurrentProgress	progress;
	Config*					config_ptr;
	Server const *			server_ptr;
	Locate const *			locate_ptr;
	int						file_fd;
	int						cgi_input_fd;
	int						cgi_output_fd;
};

#endif
