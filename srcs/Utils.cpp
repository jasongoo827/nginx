#include "Utils.hpp"

namespace utils
{
	std::vector<std::string> SplitToVector(std::string& s)
	{
		std::istringstream iss(s);
		std::string token;
		std::vector<std::string> token_vec;
		while (iss >> token)
			token_vec.push_back(token);
		return token_vec;
	}

	Status ParseVariable(std::string& dst, std::string& src)
	{
		std::vector<std::string> token_vec = SplitToVector(src);
		if (token_vec.size() != 2)
			return Status::Error("Parsing error");
		dst = token_vec.back();
		return Status::OK();
	}

	Status ParseVariable(int& dst, std::string& src)
	{
		std::vector<std::string> token_vec = SplitToVector(src);
		if (token_vec.size() != 2 || !IsStrDigit(token_vec.back()))
			return Status::Error("Parsing error");
		dst = stoi(token_vec.back());
		return Status::OK();
	}

	Status ParseVariable(std::map<int, std::string>& dst, std::string& src)
	{
		std::vector<std::string> token_vec = SplitToVector(src);
		if (token_vec.size() <= 2)
			return Status::Error("Parsing error");
		for (size_t i = 1; i < token_vec.size() - 1; ++i)
		{
			if (!IsStrDigit(token_vec[i]))
				return Status::Error("Parsing error");
			dst.insert(std::make_pair(stoi(token_vec[i]), token_vec.back()));
		}
		return Status::OK();
	}

	Status 	ParseVariable(std::vector<std::string>& dst, std::string& src, std::string& cmp)
	{
		std::vector<std::string> token_vec = SplitToVector(src);
		if (token_vec.size() < 2)
			return Status::Error("Parsing error");
		if (token_vec.front() != cmp)
			return Status::Error("Parsing error");
		for (size_t i = 1; i < token_vec.size(); ++i)
			dst.push_back(token_vec[i]);
		return Status::OK();
	}

	Status 	ParseVariable(std::pair<int, std::string>& dst, std::string& src)
	{
		std::vector<std::string> token_vec = SplitToVector(src);
		if (token_vec.size() != 3)
			return Status::Error("Parsing error");
		if (!IsStrDigit(token_vec[1]) || (stoi(token_vec[1]) < 0 || stoi(token_vec[1]) > 999))
			return Status::Error("Parsing error");
		dst.first = stoi(token_vec[1]);
		dst.second = token_vec[2];
		return Status::OK();
	}

	bool	CheckFilePath(std::string& file_path)
	{
		struct stat statbuf;
		std::string path = "./" + file_path;
		if (stat(path.c_str(), &statbuf) != -1 && S_ISDIR(statbuf.st_mode))
			return true;
		return false;
	}

	bool IsStrDigit(std::string& s)
	{
		for (size_t i = 0; i < s.length(); ++i)
			if (!isdigit(s[i])) return false;
		return true;
	}

	bool IsStrSpace(std::string& s)
	{
		for (size_t i = 0; i < s.length(); ++i)
			if (!isspace(s[i])) return false;
		return true;
	}

	double strtod(std::string& s)
	{
		const char *str = s.c_str();
		char *ptr = NULL;
		return (std::strtod(str, &ptr));
	}

	int stoi(std::string& s)
	{
		return (atoi(s.c_str()));
	}
}

