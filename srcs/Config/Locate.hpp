#ifndef LOCATE_HPP
# define LOCATE_HPP

#include <iostream>
#include <vector>
#include <utility>
#include <sstream>

class Status;

class Locate
{
public:
	Locate();
	Locate(const Locate& other);
	Locate& operator=(const Locate& rhs);
	~Locate();

	Status 						ParseLocateBlock(std::string& locate_block);
	Status						ParseMethod(std::string& str);
	Status						ParseRoot(std::string& str);
	Status						ParseIndex(std::string& str);
	Status						ParseAutoIndex(std::string& str);
	Status						ParseFilePath(std::string& str);
	void						PrintLocateInfo(void);

	std::string					GetLocatePath(void) const;
	std::vector<std::string> 	GetMethodVec(void) const;
	std::vector<std::string> 	GetIndexVec(void) const;
	std::pair<int, std::string> GetRedirectPair(void) const;
	std::string 				GetRoot(void) const;
	std::string 				GetFilePath(void) const;
	bool 						GetAutoIndex(void) const;

private:
	std::string					locate_path;
	std::vector<std::string> 	method_vec;
	std::vector<std::string> 	index_vec;
	std::pair<int, std::string> redirect_pair;
	std::string					root;
	std::string 				file_path;
	bool						autoindex;

};

#endif