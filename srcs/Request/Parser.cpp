#include "Parser.hpp"
#include "Utils.hpp"

Parser::Parser(const std::string &buf) : data(buf) {};

Parser::~Parser(){};

void	Parser::ParseStartline(Request &request)
{
	std::string			method = "";
	std::string			url = "";
	std::string			version = "";

	method = utils::DivideStrBySpace(data);
	request.SetMethod(method);
	url = utils::DivideStrBySpace(data);
	request.SetUrl(url);
	version = utils::DivideStrByCRLF(data);
	request.SetVersion(version);
	if (request.GetMethod() == OTHER || url.empty() || version.empty())
		request.SetStatus(STARTLINE);
};

void	Parser::ParseHeader(Request &request)
{
	std::string							tmp_str = "";
	std::map<std::string, std::string>	&request_header = request.GetHeader();
	std::string							header_name;
	std::string							header_value;

	if (request.GetStatus() != NO_ERROR)
		return ;
	while (!data.empty())
	{
		tmp_str = utils::DivideStrByCRLF(data);
		if (tmp_str.empty())
			break;
		if (tmp_str[0] == ' ' || tmp_str[0] == '\t')
		{
			utils::TrimSpaceTap(tmp_str);
			request_header.rbegin()->second += " " + tmp_str;
		}
		else
			utils::SplitHeaderData(tmp_str, header_name, header_value);
		if (utils::CheckLastWhiteSpace(header_name))
		{
			std::cout << '\n' << header_name << " <<< 1\n";
			request.SetStatus(WRONG_HEADER);
		}
		request_header[header_name] = header_value;
	}
	if (request_header.empty())
	{
		std::cout << '\n' << header_name << " <<< 2\n";
		request.SetStatus(WRONG_HEADER);
	}
};

void	Parser::ParseBody(Request &request)
{
	int			data_size = 0;
	std::string	tmp_str = "";
	std::string	body = "";

	if (request.GetStatus() != NO_ERROR)
		return ;
	if (request.GetMethod() == POST)
	{
		if (request.FindValueInHeader("transfer-encoding") == "chunked")
		{
			int	cur_size = utils::ReadChunkSize(data);
			while (cur_size > 0)
			{
				tmp_str = utils::ReadData(data, cur_size);
				data_size += cur_size;
				if (tmp_str.size() != cur_size)
				{
					request.SetStatus(INVALID_CHUNK);
					break ;
				}
				cur_size = utils::ReadChunkSize(data);
				if (data_size > 1000000)
				{
					request.SetStatus(BODY_SIZE);
					break ;
				}
				body += tmp_str;
			}
			if (cur_size == -1)
				request.SetStatus(INVALID_CHUNK);
		}
		else
		{
			data_size = std::atoi(request.FindValueInHeader("content-length").c_str());
			if (request.FindValueInHeader("content-length").empty() || data_size < 0 || 1000000 < data_size)
				request.SetStatus(BODY_SIZE);
			tmp_str = utils::ReadData(data, data_size);
			if (tmp_str.size() != data_size)
				request.SetStatus(BODY_SIZE);
		}
	}
	request.SetBody(body);
};

void	Parser::ParseTrailer(Request &request)
{
	std::string							tmp_str = "";
	std::map<std::string, std::string>	&request_header = request.GetHeader();
	std::string							trailer_list = request.FindValueInHeader("trailer");
	std::string							trailer_name;
	std::string							trailer_value;

	if (request.GetStatus() != NO_ERROR)
		return ;
	while (!data.empty())
	{
		tmp_str = utils::DivideStrByCRLF(data);
		if (!tmp_str.empty())
			break;
	}
	if (tmp_str.empty())
		return ;
	if (trailer_list != "" || tmp_str.find("trailer") != std::string::npos)
	{
		if (trailer_list == "")
		{
			utils::SplitHeaderData(tmp_str, trailer_name, trailer_value);
			if (trailer_name != "trailer" || utils::CheckLastWhiteSpace(trailer_name))
			{
				std::cout << '\n' << trailer_name << " <<< 3\n";
				request.SetStatus(WRONG_HEADER);
				return ;
			}
			request_header[trailer_name] = trailer_value;
			trailer_list = trailer_value;
		}
		else
		{
			utils::SplitHeaderData(tmp_str, trailer_name, trailer_value);
			if (trailer_list.find(trailer_name) == std::string::npos || utils::CheckLastWhiteSpace(trailer_name))
			{
				std::cout << '\n' << trailer_name << " <<< 4\n";
				request.SetStatus(WRONG_HEADER);
				return ;
			}
			request_header[trailer_name] = trailer_value;
		}
		while (!data.empty())
		{
			tmp_str = utils::DivideStrByCRLF(data);
			if (tmp_str.empty())
				break;
			if (tmp_str[0] == ' ' || tmp_str[0] == '\t')
			{
				utils::TrimSpaceTap(tmp_str);
				request_header.rbegin()->second += " " + tmp_str;
			}
			else
				utils::SplitHeaderData(tmp_str, trailer_name, trailer_value);
			if (trailer_list.find(trailer_name) == std::string::npos || utils::CheckLastWhiteSpace(trailer_name))
			{
				std::cout << '\n' << trailer_name << " <<< 5\n";
				request.SetStatus(WRONG_HEADER);
				return ;
			}
			request_header[trailer_name] = trailer_value;
		}
	}
	else
	{
		std::cout << '\n' << trailer_name << " <<< 6\n";
		request.SetStatus(WRONG_HEADER);
	}
};
