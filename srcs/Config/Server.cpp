#include "Server.hpp"
#include "../Status.hpp"
#include "Locate.hpp"

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
	std::istringstream iss(server_block);
	std::string str;
	Status status;
	while (getline(iss, str, '\n'))
	{
		std::vector<std::string> token_vec;
		if (str.find("listen"))
		{
			status = utils::ParseVariable(this->port, str);
			// ushrt_max --> header 필요?
			if (status.ok() && (this->port < 0 || this->port > USHRT_MAX))
				return Status::Error("Parsing error");
		}
		else if (str.find("server_name"))
			status = utils::ParseVariable(this->server_name, str);
		else if (str.find("error_page"))
		{
			status = utils::ParseVariable(this->error_page, str);
			if (status.ok())
			{
				for (std::map<int, std::string>::iterator it = this->error_page.begin(); it != this->error_page.end(); ++it)
					if (it->first < 0 || it->first > 999) return Status::Error("Parsing error");
			}
		}
		else if (str.find("cgi"))
			status = utils::ParseVariable(this->cgi_type, str);
		else if (str.find("client_body_size"))
		{
			status = utils::ParseVariable(this->client_body_size, str);
			if (status.ok() && (this->client_body_size < 0 || this->client_body_size > 1000000))
				return Status::Error("Parsing error");
		}
		else if (str.find("location"))
		{
			Locate locate;
		}
		if (!status.ok())
			return Status::Error(status.message());
	}
	return Status::OK();
}

std::string	Server::ExtractLocateBlock(std::string& server_block)
{
	std::string start_token = "location";
	size_t start_pos = server_block.find(start_token);
	if (start_pos == std::string::npos || start_pos != server_block.rfind(start_token))
		return "";
	size_t end_pos = start_pos + start_token.length();
	int brace_count = 1;
	while (end_pos < server_block.length() && brace_count > 0)
	{
		if (server_block[end_pos] == '{')
			brace_count++;
		else if (server_block[end_pos] == '}')
			brace_count--;
		end_pos++;
	}
	if (brace_count != 0 || end_pos != server_block.length())
		return "";
	return server_block.substr(start_pos, end_pos - start_pos);
}
