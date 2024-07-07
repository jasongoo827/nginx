#ifndef SESSION_HPP
# define SESSION_HPP

# include "Utils.hpp"

class Session
{
public:
				Session();
				~Session();

	bool		CheckValidSession(std::string cli_cookie);
	void		UpdateExpireTime();
	std::string	&CreateSession();
	void		AddSessionData(std::string file_name);
	bool		CheckAuth(std::string file_name);

private:
	std::string									send_cookie;
	size_t										hash_cookie;
	std::map<size_t, size_t>					expire_time;
	std::map<size_t, std::vector<std::string>&>	sessions;

	size_t		GetTimeMicro();
	void		SetHashCookie(std::string &cli_cookie_str);
	void		GenerateCookie();
				Session(const Session &rhs);
	Session&	operator=(const Session &rhs);
};

#endif
