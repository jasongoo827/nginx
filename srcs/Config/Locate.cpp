#include "Locate.hpp"
#include "../Status.hpp"
#include "../Utils.hpp"

Locate::Locate(): autoindex(false), dup_mask(0) {}

Locate::Locate(const Locate& other): locate_path(other.locate_path), method_vec(other.method_vec),
index_vec(other.index_vec), redirect_pair(other.redirect_pair), root(other.root), file_path(other.file_path),
autoindex(other.autoindex), dup_mask(other.dup_mask) {}

Locate& Locate::operator=(const Locate& rhs)
{
	if (this == &rhs)
		return (*this);
	locate_path = rhs.locate_path;
	method_vec = rhs.method_vec;
	index_vec = rhs.index_vec;
	redirect_pair = rhs.redirect_pair;
	root = rhs.root;
	file_path = rhs.file_path;
	autoindex = rhs.autoindex;
	dup_mask = rhs.dup_mask;
	return (*this);
}

Locate::~Locate()
{
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
		if (str[str.length() - 1] != ';')
			return Status::Error("Parsing error");
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
		else if (utils::find(str, "filepath"))
			status = ParseFilePath(str);
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
	Status status = utils::ParseVariable(this->method_vec, str, cmp);
	if (status.ok())
	{
		for (size_t i = 0; i < this->method_vec.size(); ++i)
		{
			if (method_vec[i] != "POST" && method_vec[i] != "GET" && method_vec[i] != "DELETE")
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

Status	Locate::ParseFilePath(std::string& str)
{
	if (dup_mask & FILEPATH)
		return Status::Error("filepath duplicate error");
	dup_mask |= FILEPATH;
	// if (status.ok() /*&& !utils::CheckFilePath(this->root)*/)
	// 	return Status::Error("Parsing error");
	return utils::ParseVariable(this->file_path, str);
}

const std::string&	Locate::GetLocatePath(void) const
{
	return locate_path;
}

const std::vector<std::string>& Locate::GetMethodVec(void) const
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

const std::string& Locate::GetFilePath(void) const
{
	return file_path;
}

bool Locate::GetAutoIndex(void) const
{
	return autoindex;
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
	std::cout << "filepath: " << this->file_path;
	std::cout << '\n';
}