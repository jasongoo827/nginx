#include "Locate.hpp"
#include "../Status.hpp"
#include "../Utils.hpp"

Locate::Locate(): autoindex(false) {}

Locate::Locate(const Locate& other): locate_path(other.locate_path), method_vec(other.method_vec),
index_vec(other.index_vec), redirect_pair(other.redirect_pair), root(other.root), file_path(other.file_path),
autoindex(other.autoindex) {}

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
	return (*this);
}

Locate::~Locate()
{
	method_vec.clear();
	index_vec.clear();
}

Status Locate::ParseLocateBlock(std::istringstream& iss, std::string& locate_block)
{
	// std::cout << "Locate::ParseLocateBlock\n";
	// std::istringstream iss(locate_block);
	std::string str;
	Status status;
	(void)locate_block;
	while (getline(iss, str, '\n'))
	{
		// std::cout << str << '\n';
		if (str.find('#') != std::string::npos || str.empty() || utils::IsStrSpace(str)) continue;
		if (str.find("}") != std::string::npos) break;
		if (str[str.length() - 1] != ';') return Status::Error("Parsing error");
		else str.resize(str.length() - 1);
		if (str.find("limit_except") != std::string::npos)
		{
			std::string cmp("limit_except");
			status = utils::ParseVariable(this->method_vec, str, cmp);
			if (status.ok())
			{
				for (size_t i = 0; i < this->method_vec.size(); ++i)
				{
					// std::cout << method_vec[i];
					if (method_vec[i] != "POST" && method_vec[i] != "GET" && method_vec[i] != "DELETE")
						return Status::Error("wrong method error");
				}
			}
		}
		else if (str.find("return") != std::string::npos)
			status = utils::ParseVariable(this->redirect_pair, str);
		else if (str.find("root") != std::string::npos)
		{
			status = utils::ParseVariable(this->root, str);
			if (status.ok() && !utils::CheckFilePath(this->root))
				return Status::Error("file path error");
		}
		else if (str.find("index") != std::string::npos && str.find("auto") == std::string::npos)
		{
			std::string cmp("index");
			status = utils::ParseVariable(this->index_vec, str, cmp);
		}
		else if (str.find("autoindex") != std::string::npos)
		{
			std::string tmp;
			status = utils::ParseVariable(tmp, str);
			std::cout << tmp << '\n';
			if (status.ok())
			{if (tmp == "ON" || tmp == "on") this->autoindex = true;
			else if (tmp == "OFF" || tmp == "off") this->autoindex = false;
			else return Status::Error("Parsing error");}
		}
		else if (str.find("filepath") != std::string::npos)
		{
			status = utils::ParseVariable(this->file_path, str);
			if (status.ok() && !utils::CheckFilePath(this->root))
				return Status::Error("Parsing error");
		}
		if (!status.ok())
			return Status::Error("Parsing error");
	}
	return Status::OK();
}

void Locate::PrintLocateInfo(void)
{
	std::cout << "\nLOCATION INFO" << '\n';
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