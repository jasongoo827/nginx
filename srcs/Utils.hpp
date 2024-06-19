#ifndef UTILS_HPP
# define UTILS_HPP

#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <utility>
#include <stdlib.h>
#include <sys/stat.h>
#include "Status.hpp"

namespace utils
{
	std::vector<std::string> 	SplitToVector(std::string& s);
	Status 						ParseVariable(std::string& dst, std::string& src);
	Status 						ParseVariable(int& dst, std::string& src);
	Status 						ParseVariable(std::map<int, std::string>& dst, std::string& src);
	Status 						ParseVariable(std::vector<std::string>& dst, std::string& src, std::string& cmp);
	Status 						ParseVariable(std::pair<int, std::string>& dst, std::string& src);
	bool						CheckFilePath(std::string& file_path);
	bool						CheckExtension(std::string& file, const char *ex);
	bool						find(std::string& dst, const char *src);
	bool 						IsStrDigit(std::string& s);
	bool						IsStrSpace(std::string& s);
	double						strtod(std::string& s);
	int 						stoi(std::string& s);
}

#endif
