#ifndef UTILS_HPP
# define UTILS_HPP

# include <iostream>
# include <vector>
# include <map>
# include <sstream>
# include <utility>
# include <stdlib.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <ctime>
# include "Status.hpp"
# include "Enum.hpp"

// Svr Include
# include <fcntl.h>
# include <string.h>
# include <unistd.h>
# include <arpa/inet.h>
# include <sys/socket.h>
# include <sys/event.h>

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
	size_t						hstoi(const std::string &trgt);
	size_t						ReadChunkSize(std::string &data);
	std::string					ReadData(std::string &data, size_t size);
	bool						CheckNameChar(std::string &data);
	bool						CheckHostDup(std::string &header_name, std::map<std::string, std::string> &request_header);
	void						SplitHeaderData(std::string &data, std::string &name, std::string &value);
	bool						SetNonBlock(int &sock_trgt);
	std::string					MethodToString(enum Method method);
}

#endif
