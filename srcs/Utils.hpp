#ifndef UTILS_HPP
# define UTILS_HPP

#include <iostream>
#include <vector>
#include <sstream>
#include <stdlib.h>

namespace utils
{
	std::vector<std::string>& 	SplitToVector(std::string& s);
	double						strtod(std::string& s);
}

#endif
