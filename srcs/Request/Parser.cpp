#include "Parser.hpp"
#include "Utils.hpp"

Parser::Parser() : data("") {};

Parser::~Parser(){};

std::string&	Parser::GetData(){
	return data;
};

void	Parser::ParseStartline(Request &request)
{
	std::string			method = "";
	std::string			url = "";
	std::string			version = "";

	if (request.GetStatus() != READ_STARTLINE)
		return ;
	method = utils::DivideStrBySpace(data);
	request.SetMethod(method);
	url = utils::DivideStrBySpace(data);
	request.SetUrl(url);
	version = utils::DivideStrByCRLF(data);
	request.SetVersion(version);
	request.SetStatus(READ_HEADER);
	if (request.GetMethod() == EMPTY || url.empty() || version.empty())
		request.SetStatus(BAD_REQUEST);
};

void	Parser::ParseHeader(Request &request)
{
	std::string							tmp_str = "";
	std::map<std::string, std::string>	&request_header = request.GetHeader();
	std::string							header_name = "";
	std::string							header_value = "";

	if (request.GetStatus() != READ_HEADER)
		return ;
	if (data.find("\r\n\r\n") != std::string::npos)
		request.SetStatus(READ_BODY);
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
		if (utils::CheckNameChar(header_name) || utils::CheckHostDup(header_name, request_header))
		{
			request.SetStatus(BAD_REQUEST);
			return ;
		}
		request_header.insert(std::make_pair(header_name, header_value));
	}
	if (request_header.empty() || request_header.find("host") == request_header.end())
		request.SetStatus(BAD_REQUEST);
};

void	Parser::ParseBody(Request &request)
{
	size_t		data_size = 0;
	std::string	tmp_str = "";
	std::string	body = request.GetBody();

	if (request.GetStatus() != READ_BODY)
		return ;
	if (request.GetMethod() == POST)
	{
		if (request.FindValueInHeader("transfer-encoding") == "chunked")
		{
			int	cur_size = request.GetBytesToRead();
			if (cur_size == 0)
				cur_size = utils::ReadChunkSize(data);
			while (cur_size > 0)
			{
				if (data_size > 10000000 || cur_size > 10000000)
				{
					request.SetStatus(BAD_REQUEST);
					return ;
				}
				tmp_str = utils::ReadData(data, cur_size);
				data_size += cur_size;
				body += tmp_str;
				if (tmp_str.size() != static_cast<size_t>(cur_size))
				{
					request.SetBytesToRead(cur_size - tmp_str.size());
					break ;
				}
				cur_size = utils::ReadChunkSize(data);
			}
			if (cur_size == 0)
				request.SetStatus(READ_TRAILER);
		}
		else
		{
			if (request.GetBytesToRead() == 0)
				request.SetBytesToRead(std::atoi(request.FindValueInHeader("content-length").c_str()));
			data_size = request.GetBytesToRead();
			if (request.FindValueInHeader("content-length").empty() || data_size < 0 || 10000000 < data_size)
			{
				request.SetStatus(BAD_REQUEST);
				return ;
			}
			tmp_str = utils::ReadData(data, data_size);
			request.SetBytesToRead(data_size - tmp_str.size());
			body += tmp_str;
			if (request.GetBytesToRead() == 0)
				request.SetStatus(READ_DONE);
		}
	}
	else
		request.SetStatus(READ_DONE);
	request.SetBody(body);
};

void	Parser::ParseTrailer(Request &request)
{
	std::string							tmp_str = "";
	std::map<std::string, std::string>	&request_header = request.GetHeader();
	std::string							trailer_list = request.FindValueInHeader("trailer");
	std::string							trailer_name;
	std::string							trailer_value;

	if (request.GetStatus() != READ_TRAILER)
		return ;
	if (data.find("\r\n\r\n") != std::string::npos)
		request.SetStatus(READ_DONE);
	while (!data.empty())
	{
		tmp_str = utils::DivideStrByCRLF(data);
		if (!tmp_str.empty())
			break;
	}
	if (tmp_str.empty() || trailer_list == "")
	{
		request.SetStatus(READ_DONE);
		return ;
	}
	utils::SplitHeaderData(tmp_str, trailer_name, trailer_value);
	if (trailer_list.find(trailer_name) == std::string::npos || utils::CheckNameChar(trailer_name))
	{
		request.SetStatus(BAD_REQUEST);
		return ;
	}
	if (trailer_name == "host" && request_header.find("host") != request_header.end())
	{
		request.SetStatus(BAD_REQUEST);
		return ;
	}
	request_header.insert(std::make_pair(trailer_name, trailer_value));
	while (!data.empty())
	{
		tmp_str = utils::DivideStrByCRLF(data);
		if (tmp_str.empty())
			break ;
		if (tmp_str[0] == ' ' || tmp_str[0] == '\t')
		{
			utils::TrimSpaceTap(tmp_str);
			request_header.rbegin()->second += " " + tmp_str;
		}
		else
			utils::SplitHeaderData(tmp_str, trailer_name, trailer_value);
		if (trailer_list.find(trailer_name) == std::string::npos || utils::CheckNameChar(trailer_name))
		{
			request.SetStatus(BAD_REQUEST);
			return ;
		}
		if (trailer_name == "host" && request_header.find("host") != request_header.end())
		{
			request.SetStatus(BAD_REQUEST);
			return ;
		}
		request_header.insert(std::make_pair(trailer_name, trailer_value));
	}
};
