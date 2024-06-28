#ifndef CGI_HPP
# define CGI_HPP

# include <Request.hpp>
# include <Server.hpp>
# include <map>

class Cgi
{
public:
	Cgi();
	Cgi(const Cgi& ref);
	~Cgi();
	Cgi& operator=(const Cgi& ref);

	void	setEnv(const Request& req, const Server& ser);
	void	setPipe();
	void	cgiExec();
	void	makeEnvp();


private:
	std::map<std::string, std::string>		envmap;
	std::vector<char*>						envp;
	int										pipe_in[2];
	int										pipe_out[2];
};


#endif
