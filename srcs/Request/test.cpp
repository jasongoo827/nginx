#include "Request.hpp"
#include "Parser.hpp"

int main(int argc, char **argv)
{
	std::string	test_buf = "POST / HTTP/1.1\r\nHost:     google.com    \r\nTransfer-encoding: chunked\r\nName:    dongyeuk    2    \r\nAge:   30    \r\nhobby:    game     \r\ntrailer:  trailer-conetent\r\n\r\n3\r\nabc\r\nd\r\nabcdefghijklm\r\n5\r\nabcde\r\n0\r\n\r\ntrailer-conetent:~~~~~asiofjaoiswfjoi~";
	Parser pars_buf(test_buf);
	Request req_1;
	pars_buf.parse_startline(req_1);
	pars_buf.parse_header(req_1);
	pars_buf.parse_body(req_1);
	pars_buf.parse_trailer(req_1);
	std::cout << req_1.get_method() << " " << req_1.get_url() << " " << req_1.get_version() << '\n';
	std::map<std::string, std::string> tmp_map = req_1.get_header();
	for (std::map<std::string, std::string>::iterator it = tmp_map.begin(); it != tmp_map.end(); ++it)
		std::cout << it->first << ": \'" << it->second << "\'\n";
	std::cout << '\'' << req_1.get_body() << "\'\n";
	std::cout << "status = " << req_1.get_status() << '\n';
	return 0;
}