#include "Locate.hpp"
#include "Status.hpp"
#include "Utils.hpp"

Locate::Locate():  client_body_size(150000000), autoindex(false), dup_mask(0) {}

Locate::Locate(const Locate& other): locate_path(other.locate_path), method_vec(other.method_vec),
index_vec(other.index_vec), redirect_pair(other.redirect_pair), cgi_map(other.cgi_map), root(other.root),
client_body_size(other.client_body_size), autoindex(other.autoindex), dup_mask(other.dup_mask) {}

Locate& Locate::operator=(const Locate& rhs)
{
	if (this == &rhs)
		return (*this);
	locate_path = rhs.locate_path;
	method_vec = rhs.method_vec;
	index_vec = rhs.index_vec;
	redirect_pair = rhs.redirect_pair;
	cgi_map = rhs.cgi_map;
	root = rhs.root;
	client_body_size = rhs.client_body_size;
	autoindex = rhs.autoindex;
	dup_mask = rhs.dup_mask;
	return (*this);
}

Locate::~Locate()
{
	cgi_map.clear();
	method_vec.clear();
	index_vec.clear();
}

Status Locate::ParseLocateBlock(std::string& locate_block)
{
	// std::cout << "Locate::ParseLocateBlock\n";
	std::istringstream iss(locate_block);
	std::string str;
	Status status;
	while (getline(iss, str, '\n'))
	{
		// std::cout << str << '\n';
		if (str.find('#') != std::string::npos || str.empty() || utils::IsStrSpace(str))
			continue;
		if (!utils::CheckTerminator(str))
			return Status::Error("Terminator error");
		else
			str.resize(str.length() - 1);
		if (utils::find(str, "limit_except"))
			status = ParseMethod(str);
		else if (utils::find(str, "return"))
			status = ParseRedirect(str);
		else if (utils::find(str, "root"))
			status = ParseRoot(str);
		else if (utils::find(str, "index"))
			status = ParseIndex(str);
		else if (utils::find(str, "autoindex"))
			status = ParseAutoIndex(str);
		else if (utils::find(str, "cgi"))
			status = ParseCgi(str);
		else if (utils::find(str, "client_body_size"))
			status = ParseClientSize(str);
		else
			return Status::Error("wrong config option error");
		if (!status.ok())
			return Status::Error("Parsing error");
	}
	return Status::OK();
}

Status	Locate::ParseMethod(std::string& str)
{
	if (dup_mask & METHOD)
		return Status::Error("method duplicate error");
	dup_mask |= METHOD;
	std::string cmp("limit_except");
	std::vector<std::string> tmp_method_vec;
	Status status = utils::ParseVariable(tmp_method_vec, str, cmp);
	if (status.ok())
	{
		for (size_t i = 0; i < tmp_method_vec.size(); ++i)
		{
			if (tmp_method_vec[i] == "POST")
				method_vec.push_back(POST);
			else if (tmp_method_vec[i] == "GET")
				method_vec.push_back(GET);
			else if (tmp_method_vec[i] == "DELETE")
				method_vec.push_back(DELETE);
			else
				return Status::Error("wrong method error");
		}
	}
	return status;
}

Status Locate::ParseRedirect(std::string& str)
{
	if (dup_mask & REDIRECT)
		return Status::Error("redirect duplicate error");
	dup_mask |= REDIRECT;
	return utils::ParseVariable(this->redirect_pair, str);
}

Status	Locate::ParseRoot(std::string& str)
{
	if (dup_mask & ROOT)
		return Status::Error("root duplicate error");
	dup_mask |= ROOT;
	// if (status.ok()/* && !utils::CheckFilePath(this->root)*/)
	// 	return Status::Error("file path error");
	return utils::ParseVariable(this->root, str);
}

Status	Locate::ParseIndex(std::string& str)
{
	if (dup_mask & INDEX)
		return Status::Error("index duplicate error");
	dup_mask |= INDEX;
	std::string cmp("index");
	return utils::ParseVariable(this->index_vec, str, cmp);
}

Status	Locate::ParseAutoIndex(std::string& str)
{
	if (dup_mask & AUTOINDEX)
		return Status::Error("autoindex duplicate error");
	dup_mask |= AUTOINDEX;
	std::string tmp;
	Status status = utils::ParseVariable(tmp, str);
	if (status.ok())
	{
		if (tmp == "ON" || tmp == "on")
			this->autoindex = true;
		else if (tmp == "OFF" || tmp == "off")
			this->autoindex = false;
		else
			return Status::Error("Parsing error");
	}
	return status;
}

Status	Locate::ParseClientSize(std::string& str)
{
	if (dup_mask & CLIENT_SIZE)
		return Status::Error("client size duplicate error");
	dup_mask |= CLIENT_SIZE;
	Status status = utils::ParseVariable(this->client_body_size, str);
	if (status.ok() && (this->client_body_size < 0 || this->client_body_size > 1000000 || this->client_body_size > std::numeric_limits<int>::max() || this->client_body_size < std::numeric_limits<int>::min()))
		return Status::Error("Parsing error");
	return status;
}

Status Locate::ParseCgi(std::string& str)
{
	if (dup_mask & CGI_EXT)
		return Status::Error("cgi duplicate error");
	dup_mask |= CGI_EXT;
	return utils::ParseVariable(this->cgi_map, str);
}

// Status	Locate::ParseFilePath(std::string& str)
// {
// 	if (dup_mask & FILEPATH)
// 		return Status::Error("filepath duplicate error");
// 	dup_mask |= FILEPATH;
// 	// if (status.ok() /*&& !utils::CheckFilePath(this->root)*/)
// 	// 	return Status::Error("Parsing error");
// 	return utils::ParseVariable(this->file_path, str);
// }

const std::string&	Locate::GetLocatePath(void) const
{
	return locate_path;
}

const std::vector<enum Method>& Locate::GetMethodVec(void) const
{
	return method_vec;
}

const std::vector<std::string>& Locate::GetIndexVec(void) const
{
	return index_vec;
}

const std::pair<int, std::string>& Locate::GetRedirectPair(void) const
{
	return redirect_pair;
}

const std::string& Locate::GetRoot(void) const
{
	return root;
}

const std::map<std::string, std::string>& Locate::GetCgiMap(void) const
{
	return cgi_map;
}

bool Locate::GetAutoIndex(void) const
{
	return autoindex;
}

const ssize_t& Locate::GetClientBodySize(void) const
{
	return client_body_size;
}

void Locate::SetLocatePath(std::string& str)
{
	std::string path = "";
	for (size_t i = strlen("\tlocation /"); str[i] != ' '; ++i)
		path += str[i];
	this->locate_path = "/" + path;
}

void Locate::PrintLocateInfo(void)
{
	std::cout << "\nLOCATION INFO" << '\n';
	std::cout << "Locate Path: " << locate_path << '\n';
	std::cout << "method list: ";
	for (size_t i = 0; i < this->method_vec.size(); ++i)
		std::cout << this->method_vec[i] << " ";
	std::cout << '\n';
	std::cout << "return " << this->redirect_pair.first << " " << this->redirect_pair.second << '\n';
	std::cout << "root: " << this->root << '\n';
	std::cout << "index: ";
	for (size_t i = 0; i < this->index_vec.size(); ++i)
		std::cout << this->index_vec[i] << " ";
	std::cout << '\n';
	std::cout << "autoindex ";
	if (this->autoindex) std::cout << "ON\n";
	else std::cout << "OFF\n";
	std::cout << "client body size: " << this->client_body_size << "\n";
	std::cout << "cgi : ";
	for (std::map<std::string, std::string>::iterator it = cgi_map.begin(); it != cgi_map.end(); ++it)
		std::cout << it->first << ":" << it->second << ' ';
	std::cout << "\n\n";
}