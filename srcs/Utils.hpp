#ifndef UTILS_HPP
# define UTILS_HPP

#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <stdlib.h>
#include "Status.hpp"

namespace utils
{
	std::vector<std::string>& 	SplitToVector(std::string& s);
	Status 						ParseVariable(std::string& dst, std::string& src);
	Status 						ParseVariable(int& dst, std::string& src);
	Status 						ParseVariable(std::map<int, std::string>& dst, std::string& src);
	bool 						IsStrDigit(std::string& s);
	double						strtod(std::string& s);
	int 						stoi(std::string& s);
}

#endif
