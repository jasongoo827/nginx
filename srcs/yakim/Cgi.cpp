#include "Cgi.hpp"
#include "ServerManager.hpp"
#include "Utils.hpp"
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <vector>

Cgi::Cgi()
{
	pipe_in[0] = 0;
	pipe_in[1] = 0;
	pipe_out[0] = 0;
	pipe_out[1] = 0;
	pid = 0;
}

Cgi::Cgi(const Cgi& ref)
{
	pipe_in[0] = 0;
	pipe_in[1] = 0;
	pipe_out[0] = 0;
	pipe_out[1] = 0;
	pid = ref.pid;
}

Cgi::~Cgi()
{
	envmap.clear();
	envp.clear();
}

Cgi& Cgi::operator=(const Cgi& ref)
{
	pipe_in[0] = 0;
	pipe_in[1] = 0;
	pipe_out[0] = 0;
	pipe_out[1] = 0;
	pid = ref.pid;
	return (*this);
}

int	Cgi::GetPipeIn()
{
	return (pipe_in[0]);
}

int	Cgi::GetPipeOut()
{
	return (pipe_out[1]);
}

int	Cgi::GetPid()
{
	return (pid);
}

void	Cgi::SetPid(int pid)
{
	this->pid = pid;
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

	envmap[std::string("USER")] = std::string(std::getenv("USER"));
	envmap[std::string("PATH")] = std::string(std::getenv("PATH"));
	envmap[std::string("LANG")] = std::string(std::getenv("LANG"));
	envmap[std::string("PWD")] = std::string(std::getenv("PWD"));

	envmap[std::string("AUTH_TYPE")] = std::string("");
	envmap[std::string("CONTENT_LENGTH")] = req.GetHeader()["content-length"];
	envmap[std::string("CONTENT_TYPE")] = req.GetHeader()["content-type"];
	envmap[std::string("GATEWAY_INTERFACE")] = std::string("CGI/1.1");
	envmap[std::string("PATH_INFO")] = std::string(req.GetUrl());
	envmap[std::string("PATH_TRANSLATED")] = std::string(req.GetUrl());//
	envmap[std::string("PATH_FILEUPLOAD")] = std::string(ser.GetFilePath());//
	envmap[std::string("QUERY_STRING")] = "";
	envmap[std::string("REMOTE_ADDR")] = "127.0.0.1";
	envmap[std::string("REQUEST_METHOD")] = utils::MethodToString(req.GetMethod());
	envmap[std::string("SCRIPT_NAME")] = "";//
	envmap[std::string("SERVER_NAME")] = ser.GetServerName();
	envmap[std::string("SERVER_PORT")] = ser.GetPort();
	envmap[std::string("SERVER_PROTOCOL")] = std::string("HTTP/1.1");
	envmap[std::string("SERVER_SOFTWARE")] = std::string("nginx/0.1");
	envmap[std::string("HTTP_X_SECRET_HEADER_FOR_TEST")] = req.GetHeader()["x-secret-header-for-test"];
	
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
	std::cout << "pipein: " << pipe_in[0] << pipe_in[1] << "\n";
	std::cout << "pipeout: " << pipe_out[0] << pipe_out[1] << "\n";
	return (Status::OK());
}

Status	Cgi::CgiExec(std::string& path)
{
	pid_t pid = fork();
	if (pid < 0)
	{
		return Status::Error("error");
	}
	if (pid == 0)
	{
		dup2(pipe_in[1], STDOUT_FILENO);
		dup2(pipe_out[0], STDIN_FILENO);
		close(pipe_in[0]);
		close(pipe_in[1]);
		close(pipe_out[0]);
		close(pipe_out[1]);
		char* executable = new char[path.size() + 1];
		std::strcpy(executable, path.c_str());

		char* argv[2];
		argv[0] = executable;
		argv[1] = NULL;
		if (execve(argv[0], argv, envp.data()) == -1)
		{
			std::cerr << "execve failed: errno: " << errno << "\n";
			delete[] executable;
			std::exit(1);
		}
	}
	else
	{
		close(pipe_in[1]);
		close(pipe_out[0]);
		this->pid = pid;
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

void    Cgi::Cleaner()
{
	size_t	size = envp.size();
    for (size_t i = 0; i < size; i++)
	{
		free(envp.back());
		envp.pop_back();
	}
    envmap.clear();
    pipe_in[0] = 0;
    pipe_in[1] = 0;
    pipe_in[0] = 0;
    pipe_in[1] = 0;
	pid = 0;
}
