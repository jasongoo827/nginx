#include "Session.hpp"

Session::Session(){};

Session::~Session(){};

bool	Session::CheckValidSession(std::string cli_cookie)
{
	size_t	cur_time = GetTimeMicro();

	size_t	pos = cli_cookie.find('=');
	if (pos == std::string::npos)
		return (false);
	std::string	cookie = cli_cookie.substr(cli_cookie.find('=') + 1);
	// std::cout << "\n\n" << cookie << "\n\n";
	SetHashCookie(cookie);
	if (expire_time.find(hash_cookie) == expire_time.end())
		return (false);
	if (cur_time - expire_time[hash_cookie] > 900000000)
	{
		sessions.erase(sessions.find(hash_cookie));
		expire_time.erase(expire_time.find(hash_cookie));
		return (false);
	}
	send_cookie = cookie;
	SetSendCookie();
	return (true);
}

void	Session::UpdateExpireTime()
{
	expire_time[hash_cookie] = GetTimeMicro();
}

void	Session::CreateSession()
{
	std::vector<std::string>	tmp_vec;

	GenerateCookie();
	sessions[hash_cookie] = tmp_vec;
	// 새로운 세션을 만들고 쿠키 아이디를 생성한다.
	// 쿠키 아이디를 생성하고 나서는 중복 확인을 실시한 뒤 중복인 경우 x2를 하고 다시 검사하는 식으로 구현한다.
}

std::string	&Session::GetSendCookie()
{
	return (send_cookie);
}

void	Session::AddSessionData(std::string file_name)
{
	sessions[hash_cookie].push_back(file_name);
}

bool	Session::CheckAuth(std::string file_name)
{
	for (std::vector<std::string>::iterator it = sessions[hash_cookie].begin(); it != sessions[hash_cookie].end(); ++it)
	{
		if (*it == file_name)
			return (true);
	}
	return (false);
}

size_t	Session::GetTimeMicro()
{
	struct timeval	start;

	gettimeofday(&start, NULL);
	return (start.tv_sec * 1000000 + start.tv_usec);
}

void	Session::GenerateCookie()
{
	std::stringstream	ss;
	size_t				cur_time = GetTimeMicro();

	ss << cur_time;
	send_cookie = ss.str();
	SetHashCookie(send_cookie);
	while (expire_time.find(hash_cookie) != expire_time.end())
	{
		cur_time *= 2;
		ss << cur_time;
		send_cookie = ss.str();
		SetHashCookie(send_cookie);
	}
	SetSendCookie();
}

void	Session::SetSendCookie()
{
	send_cookie = "cookieid=" + send_cookie;
	send_cookie += "; expires=";
	send_cookie += utils::getExpireTime();
	send_cookie += "; path=/; HttpOnly";

	UpdateExpireTime();
}

void	Session::SetHashCookie(std::string &cli_cookie_str)
{
	size_t	hash = 0;

	for (size_t i = 0; i < cli_cookie_str.length(); i++)
	{
		hash = hash * 7 + cli_cookie_str[i];
	}
	hash_cookie = hash;
}