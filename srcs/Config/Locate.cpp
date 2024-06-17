#include "Locate.hpp"

Locate::Locate() {}

Locate::Locate(const Locate& other) {}

Locate& Locate::operator=(const Locate& rhs) {}

Locate::~Locate()
{
	method_vec.clear();
	index_vec.clear();
}

