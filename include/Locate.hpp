#ifndef LOCATE_HPP
# define LOCATE_HPP

#include <iostream>
#include <vector>
#include <utility>
#include <sstream>
#include <map>
#include "Enum.hpp"

class Status;

class Locate
{
public:
	Locate();
	Locate(const Locate& other);
	Locate& operator=(const Locate& rhs);
	~Locate();

	Status 										ParseLocateBlock(std::string& locate_block);
	Status										ParseMethod(std::string& str);
	Status										ParseRedirect(std::string& str);
	Status										ParseRoot(std::string& str);
	Status										ParseIndex(std::string& str);
	Status										ParseAutoIndex(std::string& str);
	Status										ParseClientSize(std::string& str);
	Status										ParseCgi(std::string& str);
	bool										CheckLocateOpt(void);
	void										PrintLocateInfo(void);

	const std::string&							GetLocatePath(void) const;
	const std::vector<enum Method>&				GetMethodVec(void) const;
	const std::vector<std::string>&				GetIndexVec(void) const;
	const std::pair<int, std::string>& 			GetRedirectPair(void) const;
	const std::string& 							GetRoot(void) const;
	const std::map<std::string, std::string>& 	GetCgiMap(void) const;
	bool 										GetAutoIndex(void) const;
	const ssize_t& 								GetClientBodySize(void) const;
	void										SetLocatePath(std::string& str);

private:
	std::string							locate_path;
	std::vector<enum Method>			method_vec;
	std::vector<std::string> 			index_vec;
	std::pair<int, std::string> 		redirect_pair;
	std::map<std::string, std::string>	cgi_map;
	std::string							root;
	ssize_t								client_body_size;
	bool								autoindex;
	int									dup_mask;
};

#endif