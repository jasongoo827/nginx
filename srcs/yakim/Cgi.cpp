#include "Cgi.hpp"
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <vector>

Cgi::Cgi()
{
	
}

Cgi::Cgi(const Cgi& ref)
{

}

Cgi::~Cgi()
{
	envmap.clear();
	envp.clear();
}

Cgi& Cgi::operator=(const Cgi& ref)
{

}

/*
AUTH_TYPE:			-
CONTENT_LENGTH:		must if body exist
CONTENT_TYPE:		must if it is in header
GATEWAY_INTERFACE:	CGI/1.1
PATH_INFO:			root
PATH_TRANSLATED:	translate url-encoded
QUERY_STRING:		must exist, in our case it's: ""
REMOTE_ADDR:		must, client ip address
REQUEST_METHOD:		method
SCRIPT_NAME:		
SERVER_NAME:		config->server name
SERVER_PORT:		port
SERVER_PROTOCOL:	HTTP/1.1
SERVER_SOFTWARE:	config->version
*/

void	Cgi::setEnv(const Request& req, const Server& ser)
{
	// size_t start = root.find_last_of("/");
	// size_t finish = root.find_last_of(root);
	// only_file = root.substr(start + 1, finish - start);
	// only_root = root.substr(0, start);

	// envmap[std::string("USER")] = std::string(std::getenv("USER"));
	// envmap[std::string("PATH")] = std::string(std::getenv("PATH"));
	// envmap[std::string("LANG")] = std::string(std::getenv("LANG"));
	// envmap[std::string("PWD")] = std::string(std::getenv("PWD"));

	envmap[std::string("AUTH_TYPE")] = std::string("");
	envmap[std::string("CONTENT_LENGTH")] = std::string("");//length
	envmap[std::string("CONTENT_TYPE")] = "php-cgi";
	envmap[std::string("GATEWAY_INTERFACE")] = std::string("CGI/1.1");
	envmap[std::string("PATH_INFO")] = req.get_path();
	envmap[std::string("PATH_TRANSLATED")] = req.get_path();//
	envmap[std::string("QUERY_STRING")] = "";
	envmap[std::string("REMOTE_ADDR")] = "127.0.0.1";
	envmap[std::string("REQUEST_METHOD")] = req.get_method();
	envmap[std::string("SCRIPT_NAME")] = req.get_path();
	envmap[std::string("SERVER_NAME")] = ser.GetServerName();
	envmap[std::string("SERVER_PORT")] = ser.GetPort();
	envmap[std::string("SERVER_PROTOCOL")] = std::string("HTTP/1.1");
	envmap[std::string("SERVER_SOFTWARE")] = std::string("webserv/1.0");

	makeEnvp();
}

void	Cgi::setPipe()
{
	int flag;
	if (pipe(pipe_in) == -1)
	{
		
	}
	if (pipe(pipe_out) == -1)
	{

	}

	flag = fcntl(pipe_in[0], F_GETFL, 0);
	if (flag == -1)
	{

	}
	if (fcntl(pipe_in[0], F_SETFL, flag | O_NONBLOCK) == -1)
	{

	}
	flag = fcntl(pipe_in[1], F_GETFL, 0);
	if (flag == -1)
	{

	}
	if (fcntl(pipe_in[1], F_SETFL, flag | O_NONBLOCK) == -1)
	{

	}
	flag = fcntl(pipe_out[0], F_GETFL, 0);
	if (flag == -1)
	{

	}
	if (fcntl(pipe_out[0], F_SETFL, flag | O_NONBLOCK) == -1)
	{

	}
	flag = fcntl(pipe_out[1], F_GETFL, 0);
	if (flag == -1)
	{

	}
	if (fcntl(pipe_out[1], F_SETFL, flag | O_NONBLOCK) == -1)
	{

	}
}

void	Cgi::cgiExec()
{
	pid_t pid = fork();
	if (pid < 0)
	{
		return Status::Error();
	}
	if (pid == 0)
	{
		dup2(pipe_in[0], STDOUT_FILENO);
		dup2(pipe_out[1], STDIN_FILENO);
		close(pipe_in[0]);
		close(pipe_in[1]);
		close(pipe_out[0]);
		close(pipe_out[1]);
		char* argv[] = { strdup(path.c_str()), NULL };
		if (execve(argv[0], argv, envp.data()) == -1)
		{
			std::exit(1);
		}
	}
	else
	{
		close(pipe_in[0]);
		close(pipe_out[1]);
		ServerManager::addMap(pipe_in[1], *this);
		ServerManager::addMap(pipe_out[0], *this);
	}
}

void	Cgi::makeEnvp()
{
	std::vector<std::string> envvec;
	for (std::map<std::string, std::string>::iterator it = envmap.begin(); it != envmap.end(); ++it)
		envvec.push_back((*it).first + "=" + (*it).second);

	for (size_t i = 0; i < envvec.size(); ++i)
	{
		envp.push_back(strdup(envvec[i].c_str()));
	}
	envp.push_back(NULL);
}
