#include <iostream>
#include <string.h>

int main(void)
{
	std::string str = "location /cgi-bin {";
	std::string path = "";
	for (size_t i = strlen("location /"); str[i] != ' '; ++i)
		path += str[i];

	std::cout << path.length() << '\n';
}