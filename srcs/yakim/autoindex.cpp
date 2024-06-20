#include <string>
#include <sys/stat.h>
#include <dirent.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>

#define DIR_PATH "./" 

void autoindex()
{
	DIR* 				dirptr;
	struct dirent		*dp;
	struct stat			stat_;
	std::stringstream	str;
	std::string			tmp;

	dirptr = opendir(DIR_PATH);
	// if (dirptr == NULL)
	// {
	// 	this.response_404();
	// 	return ;
	// }
	str << "<html>\r\n<head><title>Index of " << DIR_PATH << "</title></head>\r\n";
	str << "<body>\r\n<h1>Index of " << DIR_PATH << "</h1><hr>\r\n<pre>";
	while ((dp = readdir(dirptr)) != NULL)
	{
		if (strcmp(dp->d_name, ".") == 0)//strcmp??
			continue ;
		stat(dp->d_name, &stat_);
		tmp = "";
		str << "<a href=\"" << dp->d_name << "\">" << dp->d_name << std::left << "</a>";
		if (S_ISREG(stat_.st_mode))
		{
			std::string str_tmp = std::ctime(&stat_.st_mtime);
			str_tmp.pop_back();
			str << str_tmp << stat_.st_size;
		}
		str << "\r\n";
	}
	str << "</pre><hr></body>\r\n</html>\r\n";
	closedir(dirptr);
	std::cout << str.str() << std::endl;
}

int main(void)
{
	autoindex();
}
