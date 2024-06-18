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

Status Server::ParseServerBlock(std::istringstream& iss, std::string& server_block)
{
	// std::cout << "Server::ParseServerBlock\n";
	// std::istringstream iss(server_block);
	(void)server_block;
	std::string str;
	Status status;
	while (getline(iss, str, '\n'))
	{
		// std::cout << str << '\n';
		if (str.find('#') != std::string::npos || str.empty() || utils::IsStrSpace(str)) continue;
		if (str.find("server") != std::string::npos) continue;
		if (str.find("}") != std::string::npos) break;
		// line이 ';'으로 끝나는지 검사
		if (str.find("location /") == std::string::npos && str[str.length() - 1] != ';') return Status::Error("Parsing error");
		else if (str[str.length() - 1] == ';') str.resize(str.length() - 1);
		if (str.find("listen") != std::string::npos)
		{
			status = utils::ParseVariable(this->port, str);
			// ushrt_max --> header 필요?
			if (status.ok() && (this->port < 0 || this->port > USHRT_MAX))
				return Status::Error("port number error");
		}
		else if (str.find("server_name") != std::string::npos)
			status = utils::ParseVariable(this->server_name, str);
		else if (str.find("error_page") != std::string::npos)
		{
			status = utils::ParseVariable(this->error_page, str);
			if (status.ok())
			{
				for (std::map<int, std::string>::iterator it = this->error_page.begin(); it != this->error_page.end(); ++it)
					if (it->first < 0 || it->first > 999) return Status::Error("Parsing error");
			}
		}
		else if (str.find("cgi") != std::string::npos && this->cgi_type.empty())
			status = utils::ParseVariable(this->cgi_type, str);
		else if (str.find("client_body_size") != std::string::npos)
		{
			status = utils::ParseVariable(this->client_body_size, str);
			if (status.ok() && (this->client_body_size < 0 || this->client_body_size > 1000000))
				return Status::Error("Parsing error");
		}
		else if (str.find("location /") != std::string::npos)
		{
			std::string locate_block = ExtractLocateBlock(server_block);
			if (locate_block.empty())
				return Status::Error("location block error");
			Locate locate;
			status = locate.ParseLocateBlock(iss, locate_block);
			if (status.ok())
				locate_vec.push_back(locate);
		}
		if (!status.ok())
			return Status::Error(status.message());
	}
	return Status::OK();
}

std::string	Server::ExtractLocateBlock(std::string& server_block)
{
	// std::cout << "Server::ExtractLocateBlock\n";
	std::string start_token = "location / {";
	size_t start_pos = server_block.find(start_token);
	if (start_pos == std::string::npos)
	{
		std::string path = "";
		for (size_t i = strlen("location /"); server_block[i] != ' '; ++i)
			path += server_block[i];
		start_token = "location /" + path + " {";
		start_pos = server_block.find(start_token);
		if (start_pos == std::string::npos)
			return "";
	}
	size_t end_pos = start_pos + start_token.length();
	// 엄밀한 확인 필요
	int brace_count = 1;
	while (end_pos < server_block.length() && brace_count > 0)
	{
		if (server_block[end_pos] == '{')
			brace_count++;
		else if (server_block[end_pos] == '}')
			brace_count--;
		end_pos++;
	}
	if (brace_count != 0)
		return "";
	return server_block.substr(start_pos, end_pos - start_pos);
}

void Server::PrintServerInfo(void)
{
	std::cout << "\nSERVER INFO" << '\n';
	std::cout << "listen: " << this->port << '\n';
	std::cout << "server_name: " << this->server_name << '\n';
	std::cout << "error_page: ";
	for (std::map<int, std::string>::iterator it = error_page.begin(); it != error_page.end(); ++it)
		std::cout << it->first << "  " << it->second << '\n';
	std::cout << "client_body_size: " << this->client_body_size << '\n';
	std::cout << "cgi type: " << this->cgi_type << '\n';
	for (size_t i = 0; i < this->locate_vec.size(); ++i)
		this->locate_vec[i].PrintLocateInfo();
	std::cout << '\n';
}