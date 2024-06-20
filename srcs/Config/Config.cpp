#include "Config.hpp"
#include "../Status.hpp"
#include "Server.hpp"
#include "../Utils.hpp"

Config::Config() {}

Config::Config(const Config& other): server_vec(other.server_vec), software_name(other.software_name), 
software_ver(other.software_ver), http_ver(other.http_ver), cgi_ver(other.cgi_ver) {}

Config& Config::operator=(const Config& rhs)
{
	if (this == &rhs)
		return (*this);
	server_vec = rhs.server_vec;
	software_name = rhs.software_name;
	software_ver = rhs.software_ver;
	http_ver = rhs.http_ver;
	cgi_ver = rhs.cgi_ver;
	return (*this);
}

Config::~Config()
{
	server_vec.clear();
}

Status Config::ReadConfig(std::string& file)
{
	// std::cout << "ReadConfig\n";
	if (!utils::CheckExtension(file, ".conf"))
		return Status::Error("file extension error");
	
	// config to string
	std::ifstream infile(file, std::ios::in | std::ios::binary);
	if (!infile)
		return Status::Error("Open error");
	infile.seekg(0, std::ios::end);
	size_t size = infile.tellg();
	infile.seekg(0, std::ios::beg);
	char *buffer = new char[size];
	if (!infile.read(buffer, size))
	{
		infile.close();
		return Status::Error("Read error");
	}
	infile.close();
	std::string file_content(buffer);
	delete []buffer;

	// parse config file
	Status status = ParseConfig(file_content);
	if (!status.ok())
		return Status::Error(status.message());
	return Status::OK();
}

Status Config::ParseConfig(std::string& file)
{
	// std::cout << "ParseConfig\n";
	std::istringstream iss(file);
	std::string str;
	Status status;
	while (getline(iss, str, '\n'))
	{
		// std::cout << str << '\n';
		// 중복 체크, 유효성 체크
		if (str.find('#') != std::string::npos || str.empty() || utils::IsStrSpace(str))
			continue;
		if (str.find("server") == std::string::npos && str[str.length() - 1] != ';')
			return Status::Error("Parsing error");
		if (utils::find(str, "SOFTWARE_NAME"))
			status = utils::ParseVariable(this->software_name, str);
		else if (utils::find(str, "SOFTWARE_VERSION"))
			status = utils::ParseVariable(this->software_ver, str);
		else if (utils::find(str, "HTTP_VERSION"))
			status = utils::ParseVariable(this->http_ver, str);
		else if (utils::find(str, "CGI_VERSION"))
			status = utils::ParseVariable(this->cgi_ver, str);
		else if (utils::find(str, "server"))
			status = ParseServerVariable(str, iss);
		else
			return Status::Error("wrong config option error");
		if (!status.ok())
			return Status::Error(status.message());
	}
	return Status::OK();
}

// std::string Config::ExtractServerBlock(std::string& file)
// {
// 	// std::cout << "Config::ExtractServerBlock\n";
// 	std::string start_token = "server {";
// 	size_t start_pos = file.find(start_token);
// 	if (start_pos == std::string::npos)
// 		return "";
// 	size_t end_pos = start_pos + start_token.length();
// 	// 엄밀한 확인 필요
// 	size_t eof = (start_pos == file.rfind(start_token)) ? file.length() : \
// 	file.find(start_token, start_pos + 1);
// 	int brace_count = 1;
// 	while (end_pos < eof && brace_count > 0)
// 	{
// 		if (file[end_pos] == '{')
// 			brace_count++;
// 		else if (file[end_pos] == '}')
// 			brace_count--;
// 		end_pos++;
// 	}
// 	if (brace_count != 0 || end_pos != file.length())
// 		return "";
// 	return file.substr(start_pos, end_pos - start_pos);
// }

std::string Config::ExtractServerBlock(std::istringstream& iss, std::string& first_line)
{
	if (first_line != "server {")
		return "";
	std::string str;
	std::string server_block = "";
	while (getline(iss, str, '\n') && str != "}")
	{
		server_block += str;
		server_block += "\n";
	}
	return server_block;
}

Status Config::ParseServerVariable(std::string& file, std::istringstream& iss)
{
	std::string server_block = ExtractServerBlock(iss, file);
	// std::cout << server_block << '\n';
	if (server_block.empty())
		return Status::Error("server block error");
	Server server;
	Status status = server.ParseServerBlock(server_block);
	if (status.ok())
		this->server_vec.push_back(server);
	return status;
}

const std::vector<Server>& Config::GetServerVec(void) const
{
	return server_vec;
}

const std::string&	Config::GetSoftwareName(void) const
{
	return software_name;
}

const std::string&	Config::GetSoftwareVer(void) const
{
	return software_ver;
}

const std::string&	Config::GetHttpVer(void) const
{
	return http_ver;
}

const std::string&	Config::GetCgiVer(void) const
{
	return cgi_ver;
}


void Config::PrintConfigInfo(void)
{
	std::cout << "CONFIGURATION FILE INFO" << '\n';

	std::cout << "SOFTWARE_NAME: " << this->software_name << '\n';
	std::cout << "SOFTWARE_VERSION: " << this->software_ver << '\n';
	std::cout << "HTTP_VERSION: " << this->http_ver << '\n';
	std::cout << "CGI_VERSION: " << this->cgi_ver << '\n';

	for (size_t i = 0; i < this->server_vec.size(); ++i)
		this->server_vec[i].PrintServerInfo();
}