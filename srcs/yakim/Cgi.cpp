#include "Cgi.hpp"
#include "ServerManager.hpp"
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <vector>

Cgi::Cgi()
{
	
}

Cgi::Cgi(const Cgi& ref)
{
	(void)ref;
}

Cgi::~Cgi()
{
	envmap.clear();
	envp.clear();
}

Cgi& Cgi::operator=(const Cgi& ref)
{
	(void)ref;
	return (*this);
}


int*	Cgi::GetPipeIn()
{
	return (pipe_in);
}

int*	Cgi::GetPipeOut()
{
	return (pipe_out);
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

void	Cgi::setEnv(Request& req, const Server& ser)
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
	envmap[std::string("PATH_INFO")] = std::string(req.GetUrl());
	envmap[std::string("PATH_TRANSLATED")] = std::string(req.GetUrl());//
	envmap[std::string("QUERY_STRING")] = "";
	envmap[std::string("REMOTE_ADDR")] = "127.0.0.1";
	envmap[std::string("REQUEST_METHOD")] = req.GetMethod();
	envmap[std::string("SCRIPT_NAME")] = std::string(req.GetUrl());
	envmap[std::string("SERVER_NAME")] = ser.GetServerName();
	envmap[std::string("SERVER_PORT")] = ser.GetPort();
	envmap[std::string("SERVER_PROTOCOL")] = std::string("HTTP/1.1");
	envmap[std::string("SERVER_SOFTWARE")] = std::string("nginx/0.1");

	makeEnvp();
}

Status	Cgi::setPipe()
{
	if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1)
	{
		return (Status::Error("pipe error"));
	}

	utils::SetNonBlock(pipe_in[0]);
	utils::SetNonBlock(pipe_in[1]);
	utils::SetNonBlock(pipe_out[0]);
	utils::SetNonBlock(pipe_out[1]);
	return (Status::OK());
}

Status	Cgi::CgiExec()
{
	pid_t pid = fork();
	if (pid < 0)
	{
		return Status::Error("error");
	}
	if (pid == 0)
	{
		dup2(pipe_in[0], STDOUT_FILENO);
		dup2(pipe_out[1], STDIN_FILENO);
		close(pipe_in[0]);
		close(pipe_in[1]);
		close(pipe_out[0]);
		close(pipe_out[1]);
		char* argv[2];
		std::string s("./usr/cgi/upload.php");
		argv[0] = &s[0];
		argv[1] = NULL;
		if (execve(argv[0], argv, envp.data()) == -1)
		{
			std::exit(1);
		}
	}
	else
	{
		close(pipe_in[0]);
		close(pipe_out[1]);
	}
	return (Status::OK());
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
