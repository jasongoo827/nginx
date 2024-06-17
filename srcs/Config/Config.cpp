#include <Config.hpp>
#include "../Status.hpp"

Config::Config()
{

}

Config::Config(const Config& other)
{

}

Config& Config::operator=(const Config& rhs)
{
	
}

Config::~Config()
{
	
}

Status Config::ReadConfig(std::string& file)
{
	if (!CheckExtension(file))
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

	// parsing error
	std::istringstream iss(file_content);
	std::string str;
	while (getline(iss, str, '\n'))
	{
		std::vector<std::string> token_vec;
		if (str.find("SOFTWARE_NAME"))
		{
			token_vec = utils::SplitToVector(str);
			if (token_vec.size() != 2)
				return Status::Error("Parsing error");
			this->software_name = token_vec.back();
		}
		else if (str.find("SOFTWARE_VERSION"))
		{
			token_vec = utils::SplitToVector(str);
			if (token_vec.size() != 2)
				return Status::Error("Parsing error");
			this->software_ver= token_vec.back();
		}
		else if (str.find("HTTP_VERSION"))
		{
			token_vec = utils::SplitToVector(str);
			if (token_vec.size() != 2)
				return Status::Error("Parsing error");
			this->http_ver= token_vec.back();
		}
		else if (str.find("CGI_VERSION"))
		{
			token_vec = utils::SplitToVector(str);
			if (token_vec.size() != 2)
				return Status::Error("Parsing error");
			this->cgi_ver= token_vec.back();
		}
	}
	
}

bool Config::CheckExtension(std::string& file)
{
	if (file.find(".conf") != std::string::npos && file.find(".conf") == file.rfind(".conf"))
		return true;
	return false;
}
