#include "Utils.hpp"
#include "Server.hpp"

namespace utils
{
	std::vector<std::string> SplitToVector(std::string& s)
	{
		std::istringstream iss(s);
		std::string token;
		std::vector<std::string> token_vec;
		while (iss >> token)
			token_vec.push_back(token);
		return token_vec;
	}

	std::vector<std::string> SplitToVector(std::string& s, char delimeter)
	{
		std::istringstream iss(s);
		std::string token;
		std::vector<std::string> token_vec;
		while (getline(iss, token, delimeter))
			token_vec.push_back(token);
		return token_vec;
	}

	std::vector<std::string> SplitToVector(const std::string& s, char delimeter)
	{
		std::istringstream iss(s);
		std::string token;
		std::vector<std::string> token_vec;
		while (getline(iss, token, delimeter))
			token_vec.push_back(token);
		return token_vec;
	}

	Status ParseVariable(std::string& dst, std::string& src)
	{
		std::vector<std::string> token_vec = SplitToVector(src);
		if (token_vec.size() != 2)
			return Status::Error("Parsing error");
		dst = token_vec.back();
		return Status::OK();
	}

	Status ParseVariable(ssize_t& dst, std::string& src)
	{
		std::vector<std::string> token_vec = SplitToVector(src);
		if (token_vec.size() != 2 || !IsStrDigit(token_vec.back()))
			return Status::Error("Parsing error");
		dst = stoi(token_vec.back());
		return Status::OK();
	}

	Status ParseVariable(std::map<int, std::string>& dst, std::string& src)
	{
		std::vector<std::string> token_vec = SplitToVector(src);
		if (token_vec.size() <= 2)
			return Status::Error("Parsing error");
		for (size_t i = 1; i < token_vec.size() - 1; ++i)
		{
			if (!IsStrDigit(token_vec[i]))
				return Status::Error("Parsing error");
			dst.insert(std::make_pair(stoi(token_vec[i]), token_vec.back()));
		}
		return Status::OK();
	}

	Status ParseVariable(std::map<std::string, std::string>& dst, std::string& src)
	{
		std::vector<std::string> token_vec = SplitToVector(src);
		if (token_vec.size() < 2)
			return Status::Error("Parsing error");
		for (size_t i = 1; i < token_vec.size(); ++i)
		{
			if (token_vec[i].find(':') != token_vec[i].rfind(':'))
				return Status::Error("Parsing error");
			std::vector<std::string> cgi_vec = SplitToVector(token_vec[i], ':');
			if (cgi_vec.size() != 2)
				return Status::Error("Parsing error");
			dst.insert(std::make_pair(cgi_vec.front(), cgi_vec.back()));
		}
		return Status::OK();
	}

	Status 	ParseVariable(std::vector<std::string>& dst, std::string& src, std::string& cmp)
	{
		std::vector<std::string> token_vec = SplitToVector(src);
		if (token_vec.size() < 2)
			return Status::Error("Parsing error");
		if (token_vec.front() != cmp)
			return Status::Error("Parsing error");
		for (size_t i = 1; i < token_vec.size(); ++i)
			dst.push_back(token_vec[i]);
		return Status::OK();
	}

	Status 	ParseVariable(std::pair<int, std::string>& dst, std::string& src)
	{
		std::vector<std::string> token_vec = SplitToVector(src);
		if (token_vec.size() != 3)
			return Status::Error("Parsing error");
		if (!IsStrDigit(token_vec[1]) || (stoi(token_vec[1]) < 0 || stoi(token_vec[1]) > 999))
			return Status::Error("Parsing error");
		dst.first = stoi(token_vec[1]);
		dst.second = token_vec[2];
		return Status::OK();
	}

	bool CheckFilePath(std::string& file_path)
	{
		struct stat statbuf;
		std::string path = "./" + file_path;
		if (stat(path.c_str(), &statbuf) != -1 && S_ISDIR(statbuf.st_mode))
			return true;
		return false;
	}

	bool CheckExtension(std::string& file, const char *ex)
	{
		if (file.find(ex) != std::string::npos && file.find(ex) == file.rfind(ex))
			return true;
		return false;
	}

	bool CheckIpFormat(std::string& s)
	{
		std::istringstream iss(s);
		std::string token;
		int line = 0;
		while (getline(iss, token, '.') && IsStrDigit(token))
		{
			int n = stoi(token);
			if (n < 0 || n > 255)
				return false;
			line++;
		}
		if (line != 4)
			return false;
		return true;
	}

	bool CheckTerminator(std::string& s)
	{
		if (s[s.length() - 1] != ';')
			return false;
		if (s.find(';') != s.rfind(';'))
			return false;
		return true;
	}

	bool find(std::string& dst, const char *src)
	{
		std::vector<std::string> token_vec = SplitToVector(dst);
		std::string tmp(src);
		if (token_vec.front() == tmp)
			return (true);
		return (false);
	}

	bool IsStrDigit(std::string& s)
	{
		for (size_t i = 0; i < s.length(); ++i)
			if (!isdigit(s[i])) return false;
		return true;
	}

	bool IsStrSpace(std::string& s)
	{
		for (size_t i = 0; i < s.length(); ++i)
			if (!isspace(s[i])) return false;
		return true;
	}

	double strtod(std::string& s)
	{
		const char *str = s.c_str();
		char *ptr = NULL;
		return (std::strtod(str, &ptr));
	}

	ssize_t stoi(std::string& s)
	{
		std::istringstream iss(s);
		ssize_t num;
		iss >> num;
		return (num);
	}

	std::string	getTime()
	{
		std::time_t current_time = std::time(NULL);
		std::tm* time_info = std::gmtime(&current_time);
		char buffer[100];
		std::strftime(buffer, 80, "%a, %d %b %Y %H:%M:%S GMT", time_info);
		return (buffer);
	}

	std::string	getExpireTime()
	{
		std::time_t current_time = std::time(NULL);
		std::time_t expire_time = current_time + (15 * 60);
		std::tm* time_info = std::gmtime(&expire_time);
		char buffer[100];
		std::strftime(buffer, 80, "%a, %d %b %Y %H:%M:%S GMT", time_info);
		return (buffer);
	}

	std::string	DivideStrByDoubleCRLF(std::string &data)
	{
		std::string res;

		size_t pos = data.find("\r\n\r\n");
		if (pos != std::string::npos)
		{
			res = data.substr(0, pos);
			data.erase(0, pos + 4);
		}
		else
		{
			pos = data.size();
			res = data.substr(0, pos);
			data.erase(0, data.size());
		}
		return res;
	}

	std::string	DivideNumByCRLF(std::string &data)
	{
		std::string res;
		int			size;
		size_t pos = data.find("\r\n");
		if (pos == std::string::npos)
			return ("");
		res = data.substr(0, pos);
		size = hstoi(res);
		if (size == 0)
		{
			if (data.find("\r\n\r\n") == std::string::npos)
				return ("");
			else
				data.erase(0, pos + 4);
		}
		else
			data.erase(0, pos + 2);
		return (res);
	}

	std::string	DivideStrByCRLF(std::string &data)
	{
		std::string res;

		size_t pos = data.find("\r\n");
		if (pos != std::string::npos)
		{
			res = data.substr(0, pos);
			data.erase(0, pos + 2);
		}
		else
		{
			pos = data.size();
			res = data.substr(0, pos);
			data.erase(0, data.size());
		}
		return res;
	}

	std::string	DivideStrBySpace(std::string &data)
	{
		std::string res;

		size_t pos = data.find(' ');
		if (pos != std::string::npos)
		{
			res = data.substr(0, pos);
			data.erase(0, pos + 1);
		}
		else
			res = DivideStrByCRLF(data);
		return res;
	}

	void	TrimSpaceTap(std::string &trgt)
	{
		while (trgt[0] == ' ' || trgt[0] == '\t')
			trgt.erase(0, 1);
		while (trgt[trgt.size() - 1] == ' ' || trgt[trgt.size() - 1] == '\t')
			trgt.erase(trgt.size() - 1);
	}

	int	hstoi(const std::string &trgt)
	{
		const char *tmp_str = trgt.c_str();
		int	cur_num = -1;

		while (*tmp_str != '\0')
		{
			if (std::isdigit(*tmp_str) ||
			('A' <= *tmp_str && *tmp_str <= 'F') ||
			('a' <= *tmp_str && *tmp_str <= 'f'))
			{
				if (cur_num == -1)
					cur_num = 0;
				cur_num *= 16;
				if (cur_num > 100000000)
					return (-1);
				if (std::isdigit(*tmp_str))
					cur_num += *tmp_str - 48;
				else if (std::isupper(*tmp_str))
					cur_num += *tmp_str - 55;
				else
					cur_num += *tmp_str - 87;
				tmp_str++;
			}
			else if (*tmp_str == '\r' || *tmp_str == '\n')
				tmp_str++;
			else if (*tmp_str == ';')
				break ;
			else
				return (-1);
		}
		return (cur_num);
	}

	int	ReadChunkSize(std::string &data)
	{
		std::string	res = DivideNumByCRLF(data);
		// std::cout << "res : " << res << '\n';
		int	size = 0;
		if (res.empty())
			return -1;
		size = hstoi(res);

		return size;
	}

	std::string	ReadData(std::string &data, size_t size)
	{
		std::string	tmp_str = data.substr(0, size);
		data.erase(0, size);
		if (tmp_str.find("\r\n") == std::string::npos)
			DivideStrByCRLF(data);
		return tmp_str;
	}

	bool	CheckNameChar(std::string &data)
	{
		return (data.find_first_of("()<>@,;:\\\"/[]?={} \t") != std::string::npos);
	}

	bool	CheckHostDup(std::string &header_name, std::map<std::string, std::string> &request_header)
	{
		return (header_name == "host" && request_header.find("host") != request_header.end());
	}

	void	SplitHeaderData(std::string &data, std::string &name, std::string &value)
	{
		int i = 0;

		name = data.substr(0, data.find(':'));
		value = data.substr(data.find(':') + 1);
		while (name[i] != '\0')
		{
			name[i] = std::tolower(name[i]);
			i++;
		}
		utils::TrimSpaceTap(value);
	}

	bool	SetNonBlock(int &sock_trgt)
	{
		/* 활성화 된 소켓의 기존 옵션 플래그를 가져와서 non-blocking 옵션을 추가
		통상적으로 소켓을 생성하는 과정에서 사용하는 socket, bind, listen, connect 등
		설정과 관련된 함수는 fcntl로 플래그를 설정하기 전에 선처리 해주는 것이 안전하다 */
		int	flag_trgt = fcntl(sock_trgt, F_GETFL, 0);
		if (flag_trgt == -1)
		{
			std::cerr << "fcntl - F_GETFL fail\n";
			return (false);
		}
		if (fcntl(sock_trgt, F_SETFL, flag_trgt | O_NONBLOCK) == -1)
		{
			std::cerr << "fcntl - F_SETFL fail\n";
			return (false);
		}
		return (true);
	}

	std::string	MethodToString(enum Method method)
	{
		if (method == GET)
			return "get";
		else if (method == POST)
			return "post";
		else if (method == DELETE)
			return "delete";
		else
			return "other";
	}

	void	AddWriteEvent(int kq, int client_socket_fd)
	{
		struct kevent change_event;
		EV_SET(&change_event, client_socket_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
		std::cout << "kq: " << kq << "\n";
		int ret = kevent(kq, &change_event, 1, NULL, 0, NULL);
		if (ret < 0)
		{
			std::cout << "fail to add writeevent\n";
			std::cout << errno << "\n";
		}
	}

	void	RemoveWriteEvent(int kq, int client_socket_fd)
	{
		struct kevent change_event;
		EV_SET(&change_event, client_socket_fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
		std::cout << "try to remove event: fd: " << client_socket_fd << "\n";
		int ret = kevent(kq, &change_event, 1, NULL, 0, NULL);
		if (ret < 0)
		{
			std::cout << "fail to remove writeevent\n";
			std::cout << errno << "\n";
		}
	}

	void	AddReadEvent(int kq, int fd)
	{
		struct kevent change_event;
		EV_SET(&change_event, fd, EVFILT_READ, EV_ENABLE | EV_ADD, 0, 0, NULL);
		std::cout << "kq: " << kq << "\n";
		int ret = kevent(kq, &change_event, 1, NULL, 0, NULL);
		if (ret < 0)
		{
			std::cout << "fail to add read event\n";
			std::cout << errno << "\n";
		}
	}

	void	AddReadEventForFile(int kq, int fd)
	{
		struct kevent change_event;
		EV_SET(&change_event, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
		std::cout << "kq: " << kq << "\n";
		int ret = kevent(kq, &change_event, 1, NULL, 0, NULL);
		if (ret < 0)
		{
			std::cout << "fail to add read event\n";
			std::cout << errno << "\n";
		}
	}

	void	RemoveReadEvent(int kq, int fd)
	{
		struct kevent change_event;
		EV_SET(&change_event, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
		std::cout << "kq: " << kq << "\n";
		int ret = kevent(kq, &change_event, 1, NULL, 0, NULL);
		if (ret < 0)
		{
			std::cout << "fail to remove read event\n";
			std::cout << errno << "\n";
		}
	}
}

bool	operator==(const sockaddr_in& lhs, const sockaddr_in& rhs)
{
	return	lhs.sin_family == rhs.sin_family &&
	lhs.sin_port == rhs.sin_port &&
	lhs.sin_addr.s_addr == rhs.sin_addr.s_addr;
}

bool	operator==(const std::pair<const std::__1::basic_string<char>, std::__1::basic_string<char> >& lhs, const std::__1::basic_string<char>& rhs)
{
	return	lhs.second == rhs;
}
