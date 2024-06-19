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

	Status 		ParseLocateBlock(std::string& locate_block);
	void		PrintLocateInfo(void);

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