#include "Server.hpp"
#include "Status.hpp"
#include "Locate.hpp"

Server::Server(): dup_mask(0) {}

Server::Server(const Server& other): locate_vec(other.locate_vec), error_page(other.error_page),
host_ip(other.host_ip), server_name(other.server_name), port(other.port),
file_path(other.file_path), dup_mask(other.dup_mask) {}

Server& Server::operator=(const Server& rhs)
{
	if (this == &rhs)
		return (*this);
	locate_vec = rhs.locate_vec;
	error_page = rhs.error_page;
	host_ip = rhs.host_ip;
	server_name = rhs.server_name;
	port = rhs.port;
	file_path = rhs.file_path;
	dup_mask = rhs.dup_mask;
	return (*this);
}

Server::~Server()
{
	// cgi_vec.clear();
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
		if (str.find('#') != std::string::npos || str.empty() || utils::IsStrSpace(str))
			continue;
		if (str.find("location /") == std::string::npos && !utils::CheckTerminator(str))
			return Status::Error("Terminator error");
		else if (str[str.length() - 1] == ';')
			str.resize(str.length() - 1);
		if (utils::find(str, "listen"))
			status = ParsePortVariable(str);
		else if (utils::find(str, "server_name"))
			status = ParseServerName(str);
		else if (utils::find(str, "error_page"))
			status = ParseErrorPage(str);
		else if (utils::find(str, "filepath"))
			status = ParseFilePath(str);
		else if (utils::find(str, "location"))
			status = ParseLocateVariable(str, iss);
		else
			return Status::Error("wrong config option error");
		if (!status.ok())
			return Status::Error(status.message());
	}
	if (!CheckServerOpt())
		return Status::Error("Essential server option not included");
	if (this->locate_vec.size() == 0)
		return Status::Error("no location error");
	return Status::OK();
}

Status	Server::ParsePortVariable(std::string& str)
{
	if (dup_mask & LISTEN)
		return Status::Error("port duplicate error");
	dup_mask |= LISTEN;
	std::vector<std::string> tmp_vec = utils::SplitToVector(str);
	Status status;
	if (tmp_vec.size() != 2)
		return Status::Error("wrong listen format");
	if (!utils::IsStrDigit(tmp_vec.back()))
	{
		if (tmp_vec.back().find(':') != tmp_vec.back().rfind(':'))
			return Status::Error("Parsing error");
		std::vector<std::string> ip_format = utils::SplitToVector(tmp_vec.back(), ':');
		if (!utils::CheckIpFormat(ip_format.front()))
			return Status::Error("wrong ip format");
		this->host_ip = ip_format.front();
		status = utils::ParseVariable(this->port, ip_format.back());
	}
	else
		status = utils::ParseVariable(this->port, str);
	if (status.ok() && (this->port < 0 || this->port > USHRT_MAX))
		return Status::Error("port number error");
	return status;
}

Status Server::ParseServerName(std::string& str)
{
	if (dup_mask & SERVER_NAME)
		return Status::Error("server name duplicate error");
	dup_mask |= SERVER_NAME;
	return utils::ParseVariable(this->server_name, str);
}

Status	Server::ParseErrorPage(std::string& str)
{
	Status status = utils::ParseVariable(this->error_page, str);
	if (status.ok())
	{
		for (std::map<int, std::string>::iterator it = this->error_page.begin(); it != this->error_page.end(); ++it)
			if (it->first < 300 || it->first > 599)
				return Status::Error("Parsing error");
	}
	return status;
}

Status	Server::ParseFilePath(std::string& str)
{
	if (dup_mask & FILEPATH)
		return Status::Error("filepath duplicate error");
	dup_mask |= FILEPATH;
	return utils::ParseVariable(this->file_path, str);
}

Status	Server::ParseLocateVariable(std::string& str, std::istringstream& iss)
{
	std::string locate_block = ExtractLocateBlock(iss, str);
	if (locate_block.empty())
		return Status::Error("location block error");
	Locate locate;
	locate.SetLocatePath(str);
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

bool	Server::CheckServerOpt(void)
{
	if (!(dup_mask & LISTEN))
		return false;
	if (!(dup_mask & SERVER_NAME))
		return false;
	if (!(dup_mask & FILEPATH))
		return false;
	return true;
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

const ssize_t& Server::GetPort(void) const
{
	return port;
}

const std::string& Server::GetFilePath(void) const
{
	return file_path;
}

void Server::PrintServerInfo(void)
{
	std::cout << "\nSERVER INFO" << '\n';
	std::cout << "listen: " << this->port << '\n';
	std::cout << "server_name: " << this->server_name << '\n';
	std::cout << "error_page: ";
	for (std::map<int, std::string>::iterator it = error_page.begin(); it != error_page.end(); ++it)
		std::cout << it->first << " " << it->second << "  ";
	std::cout << '\n';
	std::cout << "\nfilepath: " << this->file_path;
	for (size_t i = 0; i < this->locate_vec.size(); ++i)
		this->locate_vec[i].PrintLocateInfo();
	std::cout << "\n\n";
}