#ifndef STATUS_HPP
# define STATUS_HPP

#include <iostream>

class Status
{
public:
	static 	Status OK(void);
	static 	Status Error(const std::string& message);
	static	Status create(const std::string& message);
	bool	ok(void) const;
	const	std::string& message(void) const;
private:
	Status(const std::string& message);
	std::string _message;

};

#endif