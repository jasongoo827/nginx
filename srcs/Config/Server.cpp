#include "Server.hpp"
#include "../Status.hpp"
#include "Locate.hpp"
#include <string.h>

Server::Server() {}

Server::Server(const Server& other): locate_vec(other.locate_vec), error_page(other.error_page),
host_ip(other.host_ip), server_name(other.server_name), cgi_type(other.cgi_type), port(other.port),
client_body_size(other.client_body_size) {}

Server& Server::operator=(const Server& rhs)
{
	if (this == &rhs)
		return (*this);
	locate_vec = rhs.locate_vec;
	error_page = rhs.error_page;
	host_ip = rhs.host_ip;
	server_name = rhs.server_name;
	cgi_type = rhs.cgi_type;
	port = rhs.port;
	client_body_size = rhs.client_body_size;
	return (*this);
}

Server::~Server()
{
	locate_vec.clear();
	error_page.clear();
}

Status Server::ParseServerBlock(std::string& server_block)
{
	// std::cout << "Server::ParseServerBlock\n";
	std::istringstream iss(server_block);
	std::string str;
	Status status;
	while (getline(iss, str, '\n'))
	{
		// std::cout << str << '\n';
		if (str.find('#') != std::string::npos || str.empty() || utils::IsStrSpace(str))
			continue;
		if (str.find("location /") == std::string::npos && str[str.length() - 1] != ';')
			return Status::Error("Parsing error");
		else if (str[str.length() - 1] == ';')
			str.resize(str.length() - 1);
		if (utils::find(str, "listen"))
			status = ParsePortVariable(str);
		else if (utils::find(str, "server_name"))
			status = utils::ParseVariable(this->server_name, str);
		else if (utils::find(str, "error_page"))
			status = ParseErrorPage(str);
		else if (utils::find(str, "cgi"))
			status = utils::ParseVariable(this->cgi_type, str);
		else if (utils::find(str, "client_body_size"))
			status = ParseClientSize(str);
		else if (utils::find(str, "location"))
			status = ParseLocateVariable(str, iss);
		if (!status.ok())
			return Status::Error(status.message());
	}
	return Status::OK();
}

Status	Server::ParsePortVariable(std::string& str)
{
	std::vector<std::string> tmp_vec = utils::SplitToVector(str);
	Status status;
	if (tmp_vec.size() != 2)
		return Status::Error("wrong listen format");
	if (!utils::IsStrDigit(tmp_vec.back()))
	{
		std::vector<std::string> ip_format = utils::SplitToVector(tmp_vec.back(), ':');
		if (!utils::CheckIpFormat(ip_format.front()))
			return Status::Error("wron ip format");
		status = utils::ParseVariable(this->port, ip_format.back());
	}
	else
		status = utils::ParseVariable(this->port, str);
	// ushrt_max --> header 필요?
	if (status.ok() && (this->port < 0 || this->port > USHRT_MAX))
		return Status::Error("port number error");
	return status;
}

Status	Server::ParseErrorPage(std::string& str)
{
	Status status = utils::ParseVariable(this->error_page, str);
	if (status.ok())
	{
		for (std::map<int, std::string>::iterator it = this->error_page.begin(); it != this->error_page.end(); ++it)
			if (it->first < 0 || it->first > 999)
				return Status::Error("Parsing error");
	}
	return status;
}
Status	Server::ParseClientSize(std::string& str)
{
	Status status = utils::ParseVariable(this->client_body_size, str);
	if (status.ok() && (this->client_body_size < 0 || this->client_body_size > 1000000))
		return Status::Error("Parsing error");
	return status;
}

Status	Server::ParseLocateVariable(std::string& str, std::istringstream& iss)
{
	std::string locate_block = ExtractLocateBlock(iss, str);
	if (locate_block.empty())
		return Status::Error("location block error");
	Locate locate;
	Status status = locate.ParseLocateBlock(locate_block);
	if (status.ok())
		locate_vec.push_back(locate);
	return status;
}

std::string Server::ExtractLocateBlock(std::istringstream& iss, std::string& first_line)
{
	std::string start_token = "\tlocation / {";
	std::string path = "";
	if (first_line != start_token)
	{
		for (size_t i = strlen("\tlocation /"); first_line[i] != ' '; ++i)
			path += first_line[i];
		start_token = "\tlocation /" + path + " {";
		if (first_line != start_token)
			return "";
	}
	std::string str;
	std::string locate_block = "";
	while (getline(iss, str, '\n') && str.find("}") == std::string::npos)
	{
		locate_block += str;
		locate_block += "\n";
	}
	return locate_block;
}

const std::vector<Locate>& Server::GetLocateVec(void) const
{
	return locate_vec;
}

const std::map<int, std::string>& Server::GetErrorPage(void) const
{
	return error_page;
}

const std::string& Server::GetHostIp(void) const
{
	return host_ip;
}

const std::string& Server::GetServerName(void) const
{
	return server_name;
}

const std::string& Server::GetCgiType(void) const
{
	return cgi_type;
}

const int& Server::GetPort(void) const
{
	return port;
}

const int& Server::GetClientBodySize(void) const
{
	return client_body_size;
}

void Server::PrintServerInfo(void)
{
	std::cout << "\nSERVER INFO" << '\n';
	std::cout << "listen: " << this->port << '\n';
	std::cout << "server_name: " << this->server_name << '\n';
	for (std::map<int, std::string>::iterator it = error_page.begin(); it != error_page.end(); ++it)
		std::cout << "error_page: " << it->first << "  " << it->second << '\n';
	std::cout << "client_body_size: " << this->client_body_size << '\n';
	std::cout << "cgi type: " << this->cgi_type << '\n';
	for (size_t i = 0; i < this->locate_vec.size(); ++i)
		this->locate_vec[i].PrintLocateInfo();
	std::cout << '\n';
}