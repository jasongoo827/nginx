#ifndef CONNECTION_HPP
# define CONNECTION_HPP

# include "Response.hpp"
# include "Request.hpp"
# include "Config.hpp"
# include "Locate.hpp"

class Connection
{
public:
	Connection();
	Connection(int clinet_socket_fd, Config* config_ptr);
	Connection(const Connection& ref);
	~Connection();
	Connection& operator=(const Connection& ref);

	void	mainprocess();
	void	make_response();
	void	parserequest();
	void	process_dir();
	void	process_file();
	void	process_cgi();

private:
	int						client_socket_fd;
	Request					request;
	Response				response;
	enum current_progress	progress;
	Config*					config_ptr;
	Locate*					locate_ptr;
};

#endif
