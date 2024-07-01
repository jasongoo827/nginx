#include "Utils.hpp"

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
		int	cur_num = 0;

		while (*tmp_str != '\0')
		{
			if (std::isdigit(*tmp_str) ||
			('A' <= *tmp_str && *tmp_str <= 'F') ||
			('a' <= *tmp_str && *tmp_str <= 'f'))
			{
				cur_num *= 16;
				if (cur_num > 1000000)
					return (-1);
				if (std::isdigit(*tmp_str))
					cur_num += *tmp_str - 48;
				else if (std::isupper(*tmp_str))
					cur_num += *tmp_str - 55;
				else
					cur_num += *tmp_str - 87;
				tmp_str++;
			}
			else if (*tmp_str == ';')
				break ;
			else
				return (-1);
		}
		return (cur_num);
	}

	int	ReadChunkSize(std::string &data)
	{
		std::string	tmp_str = DivideStrByCRLF(data);
		int	size = 0;

		if (tmp_str.empty())
			return -1;
		size = hstoi(tmp_str);
		return size;
	}

	std::string	ReadData(std::string &data, int size)
	{
		std::string	tmp_str = data.substr(0, size);
		data.erase(0, size);
		if (tmp_str.find("\r\n") == std::string::npos)
			DivideStrByCRLF(data);
		return tmp_str;
	}

	int	CheckLastWhiteSpace(std::string &data)
	{
		size_t	last_pos = data.size() - 1;
		if (data.find(' ') == last_pos || data.find('\t') == last_pos)
			return 1;
		return 0;
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

	bool	InitServerAddress(sockaddr_in &addr_serv, char *port)
	{
		void	*error = memset(&addr_serv, 0, sizeof(addr_serv));
		addr_serv.sin_family = AF_INET;
		addr_serv.sin_port = htons(atoi(port));
		addr_serv.sin_addr.s_addr = htonl(INADDR_ANY);
		if (error == NULL)
		{
			std::cerr << "memset error\n";
			return (false);
		}
		return (true);
	}

	bool	InitServerSocket(int &sock_serv, sockaddr_in &addr_serv)
	{
		/* 서버가 사용할 소켓 생성 */
		sock_serv = socket(AF_INET, SOCK_STREAM, 0);
		if (sock_serv == -1)
		{
			std::cerr << "socket error\n";
			close(sock_serv);
			return (false);
		}
		/* 서버가 사용하는 소켓을 재활용 가능하도록 설정 */
		int enable = 1;
		if (setsockopt(sock_serv, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
		{
			std::cerr << "setsockopt fail\n";
			close(sock_serv);
			return (false);
		}
		/* 서버가 사용할 소켓에 서버의 정보 등록 */
		if (bind(sock_serv, (struct sockaddr*)&addr_serv, sizeof(addr_serv)) == -1)
		{
			std::cerr << "bind error\n";
			close(sock_serv);
			return (false);
		}
		/* 서버 소켓에 대한 통신 활성화 */
		if (listen(sock_serv, 16) == -1)
		{
			std::cerr << "listen error\n";
			close(sock_serv);
			return (false);
		}
		/* NonBlocking 설정 */
		if (SetNonBlock(sock_serv) == false)
		{
			std::cerr << "sock_serv nonblock error\n";
			close(sock_serv);
			return (false);
		}
		return (true);
	}

	bool	InitClientSocket(int &kq, int &sock_serv, struct ::kevent &change_event, int &sock_client, sockaddr_in &addr_client, socklen_t addr_client_len)
	{
		/* 서버 소켓으로 요청이 발생하는 경우 연결 요청이므로 신규 클라이언트 연결 */
		sock_client = accept(sock_serv, (sockaddr*)&addr_client, &addr_client_len);
		if (sock_client < 0)
		{
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                std::cout << "Connection not found\n";
                return (false);
            }
			else
			{
                std::cerr << "accept fail\n";
                return (false);
            }
        }
		/* 소켓을 재활용 가능하도록 설정 */
		int enable = 1;
		if (setsockopt(sock_serv, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
		{
			std::cerr << "setsockopt fail\n";
			close(sock_client);
			return (false);
		}
		/* NonBlocking 설정 */
		if (SetNonBlock(sock_client) == false)
		{
			std::cerr << "sock_client nonblock error\n";
			close(sock_client);
			return (false);
		}
		EV_SET(&change_event, sock_client, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
		if (::kevent(kq, &change_event, 1, NULL, 0, NULL) == -1) {
			std::cerr << "sock_serv kevent registration fail\n";
			close(sock_client);
			return (false);
		}
		return (true);
	}

	bool	InitKqueue(int &kq, int &sock_serv, struct ::kevent &change_event)
	{
		kq = kqueue();
		if (kq == -1)
		{
			std::cerr << "kqueue fail\n";
			close(sock_serv);
			return (false);
		}

		EV_SET(&change_event, sock_serv, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
		if (::kevent(kq, &change_event, 1, NULL, 0, NULL) == -1)
		{
			std::cerr << "sock_serv kevent registration fail\n";
			close(sock_serv);
			close(kq);
			return (false);
		}
		return (true);
	}

	bool	CheckEvent(int &kq, struct ::kevent *events, int &event_count, int &sock_serv)
	{
		event_count = kevent(kq, NULL, 0, events, 20, NULL); /* config에서 파싱 가능하다면 20 대신 config 설정 수치로 변경해야함 */
		if (event_count == -1)
		{
			std::cerr << "kevent wait fail\n";
			close(sock_serv);
			close(kq);
			return (false);
		}
		return (true);
	}

	void	CloseConnection(struct ::kevent &change_event, std::vector<int> &v_sock_client, std::vector<sockaddr_in> &v_addr_client, int &kq, int &sock_client, struct sockaddr_in &addr_client)
	{
		EV_SET(&change_event, sock_client, EVFILT_READ, EV_DELETE, 0, 0, NULL);
		kevent(kq, &change_event, 1, NULL, 0, NULL);
		close(sock_client);
		std::vector<int>::iterator it_sock = std::find(v_sock_client.begin(), v_sock_client.end(), sock_client);
		if (it_sock != v_sock_client.end())
			v_sock_client.erase(it_sock);
		std::vector<sockaddr_in>::iterator it_addr = std::find(v_addr_client.begin(), v_addr_client.end(), addr_client);
		if (it_addr != v_addr_client.end())
			v_addr_client.erase(it_addr);
	}

	void	CloseAllConnection(std::vector<int> &v_sock_client, std::vector<sockaddr_in> &v_addr_client, int &kq, int &sock_serv)
	{
		for (size_t i = 0; i < v_sock_client.size(); ++i)
			close(v_sock_client[i]);
		close(sock_serv);
		close(kq);
		v_sock_client.clear();
		v_addr_client.clear();
	}
}

bool	operator==(const sockaddr_in& lhs, const sockaddr_in& rhs)
	{
		return	lhs.sin_family == rhs.sin_family &&
		lhs.sin_port == rhs.sin_port &&
		lhs.sin_addr.s_addr == rhs.sin_addr.s_addr;
	}