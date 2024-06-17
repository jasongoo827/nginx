#ifndef LOCATE_HPP
# define LOCATE_HPP

#include <iostream>
#include <vector>
#include <utility>

class Locate
{
public:
	Locate();
	Locate(const Locate& other);
	Locate& operator=(const Locate& rhs);
	~Locate();

private:
	std::vector<std::string> 	method_vec;
	std::vector<std::string> 	index_vec;
	std::pair<int, std::string> redirect_pair;
	std::string					root;
	std::string 				file_path;
	bool						autoindex;

};

#endif