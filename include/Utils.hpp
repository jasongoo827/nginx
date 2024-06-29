#ifndef UTILS_HPP
# define UTILS_HPP

#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <utility>
#include <stdlib.h>
#include <sys/stat.h>
#include <ctime>
#include "Status.hpp"

namespace utils
{
	std::vector<std::string> 	SplitToVector(std::string& s);
	std::vector<std::string>	SplitToVector(std::string& s, char delimeter);
	std::vector<std::string> 	SplitToVector(const std::string& s, char delimeter);
	Status 						ParseVariable(std::string& dst, std::string& src);
	Status 						ParseVariable(ssize_t& dst, std::string& src);
	Status 						ParseVariable(std::map<int, std::string>& dst, std::string& src);
	Status 						ParseVariable(std::vector<std::string>& dst, std::string& src, std::string& cmp);
	Status 						ParseVariable(std::pair<int, std::string>& dst, std::string& src);
	bool						CheckFilePath(std::string& file_path);
	bool						CheckExtension(std::string& file, const char *ex);
	bool						CheckIpFormat(std::string& s);
	bool						find(std::string& dst, const char *src);
	bool 						IsStrDigit(std::string& s);
	bool						IsStrSpace(std::string& s);
	double						strtod(std::string& s);
	ssize_t 					stoi(std::string& s);
	std::string					getTime();
	std::string					DivideStrByCRLF(std::string &data);
	std::string					DivideStrBySpace(std::string &data);
	void						TrimSpaceTap(std::string &trgt);
	int							hstoi(const std::string &trgt);
	int							ReadChunkSize(std::string &data);
	std::string					ReadData(std::string &data, int size);
	int							CheckLastWhiteSpace(std::string &data);
	void						SplitHeaderData(std::string &data, std::string &name, std::string &value);
}

#endif
