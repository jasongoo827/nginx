#include "Parser.hpp"

static void	cast_field_name(std::string &trgt)
{
	int i = 0;

	while (trgt[i] != '\0')
	{
		trgt[i] = std::tolower(trgt[i]);
		i++;
	}
}

static void	cast_header(std::string &trgt)
{
	while (trgt[0] == ' ' || trgt[0] == '\t')
		trgt.erase(0, 1);
	while (trgt[trgt.size() - 1] == ' ' || trgt[trgt.size() - 1] == '\t')
		trgt.erase(trgt.size() - 1);
};

static int	hstoi(const std::string &trgt)
{
	const char *tmp_str = trgt.c_str();
	int	cur_num = 0;
	
	std::cout << "str in hstoi = " << trgt << '\n';
	while (*tmp_str != '\0')
	{
		if (std::isdigit(*tmp_str) ||
		('A' <= *tmp_str && *tmp_str <= 'F') ||
		('a' <= *tmp_str && *tmp_str <= 'f'))
		{
			cur_num *= 16;
			printf("%d, %d\n", *tmp_str, cur_num);
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
		else
			return (-1);
	}
	return (cur_num);
};

Parser::Parser(const std::string &buf) : data(buf) {};

Parser::~Parser(){};

bool	Parser::parse_startline(Request &request)
{
	std::string			method = "";
	std::string			url = "";
	std::string			version = "";
	std::string			tmp_line;

	std::cout << "parse_startline\n";
	std::getline(data, tmp_line);
	std::istringstream	first_line(tmp_line);
	first_line >> method;
	if (method == "GET")
		request.set_method(GET);
	else if (method == "POST")
		request.set_method(POST);
	else if (method == "DELETE")
		request.set_method(DELETE);
	else
		request.set_method(OTHER);
	first_line >> url;
	request.set_url(url);
	first_line >> version;
	request.set_version(version);
	std::cout << request.get_method() << " " << request.get_url() << " " << request.get_version() << '\n';
	// std::getline(data, tmp_line);
	// std::cout << tmp_line << '\n';
	// if (tmp_line[tmp_line.size() - 1] == '\r')
	// 	std::cout << "catch\n";
	return true; // 추후 에러 처리가 불필요할 경우 제거, 반환형 void로 변경
};

bool	Parser::parse_header(Request &request)
{
	std::string							tmp_str = "";
	std::map<std::string, std::string>	&request_header = request.get_header();
	std::string							header_name;
	std::string							header_value;

	// std::cout << "parse_header\n";
	while (std::getline(data, tmp_str) && !tmp_str.empty())
	{
		if (tmp_str[tmp_str.size() - 1] == '\r')
			tmp_str.erase(tmp_str.size() - 1);
		if (tmp_str.empty())
			break;
		// std::cout << tmp_str << '\n';
		if (tmp_str[0] == ' ' || tmp_str[0] == '\t')
		{
			cast_header(tmp_str);
			request_header.rbegin()->second += " " + tmp_str;
		}
		else
		{
			header_name = tmp_str.substr(0, tmp_str.find(':'));
			header_value = tmp_str.substr(tmp_str.find(':') + 1);
			cast_field_name(header_name);
			cast_header(header_value);
		}
		if (header_value.find(':') != std::string::npos)
			request.set_status(WRONG_HEADER);
		request_header[header_name] = header_value;
	}
	return true; // 추후 에러 처리 필요 없어지면 삭제
};

bool	Parser::parse_body(Request &request)
{
	int			data_size = 0;
	std::string	tmp_str = "";
	std::string	body = "";

	// std::cout << "parse_body\n";
	if (request.find_value_in_header("transfer-encoding") == "chunked")
	{
		int		cur_size = 0;
		bool	controler = CHUNK_SIZE;
		while (std::getline(data, tmp_str) && !tmp_str.empty())
		{
			if (tmp_str[tmp_str.size() - 1] == '\r')
				tmp_str.erase(tmp_str.size() - 1);
			if (controler == CHUNK_SIZE)
			{
				controler = CHUNK_DATA;
				cur_size = hstoi(tmp_str);
				data_size += cur_size;
				std::cout << "check2\ncur_size = " << cur_size << "\nstr : " << tmp_str << '\n';
				if (cur_size < 0)
					request.set_status(INVALID_CHUNK);
				if (cur_size < 1)
					break;
				else
					continue;
			}
			controler = CHUNK_SIZE;
			if (data_size >= 1000000)
			{
				request.set_status(BODY_SIZE);
				break;
			}
			if (tmp_str.size() != cur_size)
			{
				request.set_status(INVALID_CHUNK);
				break;
			}
			body += tmp_str;
		}
	}
	else
	{
		while (std::getline(data, tmp_str) && !tmp_str.empty())
		{
			if (!body.empty())
				body += "\n";
			if (tmp_str[tmp_str.size() - 1] == '\r')
				tmp_str.erase(tmp_str.size() - 1);
			// while (tmp_str[0] == ' ' || tmp_str[0] == '\t')
			// 	tmp_str.erase(0, 1);
			// while (tmp_str[tmp_str.size() - 1] == ' ' || tmp_str[tmp_str.size() - 1] == '\t')
			// 	tmp_str.erase(tmp_str.size() - 1);
			body += tmp_str;
			int	cur_size = tmp_str.size();
			data_size += cur_size;
			if (data_size > 1000000 || cur_size > 1000000)
			{
				request.set_status(BODY_SIZE);
				break;
			}
		}
		if (data_size != std::atoi(request.find_value_in_header("Content-Length").c_str()))
			request.set_status(BODY_SIZE);
	}
	request.set_body(body);
	// if (body.empty())
	// 	printf("no body\n");
	return true; // 추후 에러 처리 필요 없어지면 삭제
};

bool	Parser::parse_trailer(Request &request)
{
	std::string							tmp_str = "";
	std::map<std::string, std::string>	&request_header = request.get_header();
	std::string							trailer_name;
	std::string							trailer_value;

	//header에 trailer 있는지 확인
	while (std::getline(data, tmp_str) && !tmp_str.empty())
	{
		if (tmp_str[0] != '\r')
			break;
	}
	if (request.find_value_in_header("trailer") == "")
	//없으면 한 줄 읽어서 header에 저장
	{
		std::cout << "in no trailer case str : " << tmp_str << '\n';
		if (tmp_str[tmp_str.size() - 1] == '\r')
			tmp_str.erase(tmp_str.size() - 1);
		trailer_name = tmp_str.substr(0, tmp_str.find(':'));
		trailer_value = tmp_str.substr(tmp_str.find(':') + 1);
		cast_field_name(trailer_name);
		cast_header(trailer_value);
		if (trailer_value.find(':') != std::string::npos || trailer_name != "trailer")
			request.set_status(WRONG_HEADER);
		request_header[trailer_name] = trailer_value;
	}
	else
	{
		std::cout << "in no trailer case str : " << tmp_str << '\n';
		if (tmp_str[tmp_str.size() - 1] == '\r')
			tmp_str.erase(tmp_str.size() - 1);
		trailer_name = tmp_str.substr(0, tmp_str.find(':'));
		trailer_value = tmp_str.substr(tmp_str.find(':') + 1);
		cast_field_name(trailer_name);
		cast_header(trailer_value);
		if (trailer_value.find(':') != std::string::npos ||
		request.find_value_in_header("trailer").find(trailer_name) == std::string::npos)
			request.set_status(WRONG_HEADER);
		request_header[trailer_name] = trailer_value;
	}
	//저장하고 나서 다음 줄 부터 읽고 나서 name 부분이 trailer value에서 find 되는지 확인
	//기존 header에 추가
	while (std::getline(data, tmp_str) && !tmp_str.empty())
	{
		if (tmp_str[tmp_str.size() - 1] == '\r')
			tmp_str.erase(tmp_str.size() - 1);
		if (tmp_str.empty())
			break;
		// std::cout << tmp_str << '\n';
		if (tmp_str[0] == ' ' || tmp_str[0] == '\t')
		{
			cast_header(tmp_str);
			request_header.rbegin()->second += " " + tmp_str;
		}
		else
		{
			trailer_name = tmp_str.substr(0, tmp_str.find(':'));
			trailer_value = tmp_str.substr(tmp_str.find(':') + 1);\
			cast_field_name(trailer_name);
			cast_header(trailer_value);
		}
		if (trailer_value.find(':') != std::string::npos ||
		request.find_value_in_header("Trailer").find(trailer_name) == std::string::npos)
			request.set_status(WRONG_HEADER);
		request_header[trailer_name] = trailer_value;
	}
	return true; // 추후 에러 처리 필요 없어지면 삭제
};