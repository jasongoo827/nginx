#ifndef SESSION_HPP
# define SESSION_HPP

# include "Utils.hpp"

class Session
{
public:
				Session();
				~Session();

	bool		CheckValidSession(std::string cli_cookie); // ALL METHOD 항상
	void		UpdateExpireTime(); // check == true 일 때 ALL METHOD
	void		CreateSession(); // Check == false 일 때 ALL METHOD
	void		AddSessionData(std::string file_name); // POST
	bool		CheckAuth(std::string file_name); // DELETE
	void		SetSendCookie();
	std::string	&GetSendCookie(); // 헤더에 Cookie 정보 추가할 때 사용

private:
	std::string									send_cookie;
	size_t										hash_cookie;
	std::map<size_t, size_t>					expire_time;
	std::map<size_t, std::vector<std::string> >	sessions;

	size_t		GetTimeMicro();
	void		SetHashCookie(std::string &cli_cookie_str);
	void		GenerateCookie();
				Session(const Session &rhs);
	Session&	operator=(const Session &rhs);
};

#endif
