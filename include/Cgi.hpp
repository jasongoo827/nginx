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

	int	GetPipeIn();
	int	GetPipeOut();

	void	setEnv(Request& req, const Server& ser);
	Status	setPipe();
	Status	CgiExec(std::string& path);
	void	makeEnvp();
	void	Cleaner();


private:
	std::map<std::string, std::string>		envmap;
	std::vector<char*>						envp;
	int										pipe_in[2];
	int										pipe_out[2];
};


#endif
