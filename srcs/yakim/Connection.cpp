#include "Connection.hpp"
#include "Utils.hpp"
#include "Server.hpp"
#include "Parser.hpp"
#include "ServerManager.hpp"
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/event.h>
#include <sstream>
#include <iterator>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <Session.hpp>
#include <signal.h>

Connection::Connection(int kq, int clinet_socket_fd, sockaddr_in client_socket_addr, Config* config_ptr, Session* session)
: request(), response(), cgi()
{
	this->client_socket_fd = clinet_socket_fd;
	this->client_socket_addr = client_socket_addr;
	this->config_ptr = config_ptr;
	this->progress = FROM_CLIENT;
	this->file_fd = 0;
	this->pipein = 0;
	this->pipeout = 0;
	std::time(&timeval);
	this->session = session;
	this->total_len = 0;
	this->kq = kq;
}

Connection::Connection(const Connection& ref)
: request(), response(), cgi()
{
	this->client_socket_fd = ref.client_socket_fd;
	this->client_socket_addr = ref.client_socket_addr;
	this->config_ptr = ref.config_ptr;
	this->progress = ref.progress;
	this->file_fd = ref.file_fd;
	this->pipein = ref.pipein;
	this->pipeout = ref.pipeout;
	this->timeval = ref.timeval;
	this->session = ref.session;
	this->kq = ref.kq;
}

Connection::~Connection()
{

}

Connection&	Connection::operator=(const Connection& ref)
{
	this->client_socket_fd = ref.client_socket_fd;
	this->client_socket_addr = ref.client_socket_addr;
	this->config_ptr = ref.config_ptr;
	this->progress = ref.progress;
	this->file_fd = ref.file_fd;
	this->pipein = ref.pipein;
	this->pipeout = ref.pipeout;
	this->timeval = ref.timeval;
	this->request = ref.request;
	this->cgi = ref.cgi;
	this->session = ref.session;
	this->kq = ref.kq;
	return (*this);
}



void	Connection::MainProcess(struct kevent& event)
{
	std::cout << "progress: " << progress << " event.filter: " << event.filter << "\n";
	std::cout << "MainProcess for fd: " << client_socket_fd << "\n";
	std::cout << "connection progress: " << progress << "\n";
	if (progress == FROM_CLIENT && event.filter == EVFILT_READ)
	{
		ReadClient();
		if (progress == END_CONNECTION)
			return ;
		if (progress == READ_CONTINUE)
		{
			progress = FROM_CLIENT;
			return ;
		}
		// progress == readcontinue면 from_client로 바꾸고 리턴 -> 이벤트와 fd 유지하고 계속 읽기 가능!
		if (!session->CheckValidSession(this->GetRequest().FindValueInHeader("cookie")))
			session->CreateSession();
		MakeResponse();
		// if (request.FindValueInHeader("connection") == "keep-alive")
		response.AddHeader("Connection", "keep-alive");
		// else
			// response.AddHeader("Connection", "close");
		response.AddHeader("Set-Cookie", session->GetSendCookie());
		response.CombineMessage();
		return ;
	}
	else if (progress == CGI && event.filter == EVFILT_WRITE)
	{
		SendCgi();
		return ;
	}
	else if (progress == CGI && event.filter == EVFILT_READ)
	{
		ReadCgi();
		return ;
	}
	else if (progress == FROM_FILE && event.filter == EVFILT_READ)
	{
		ReadFile();
		return ;
	}
	else if (progress == TO_CLIENT && event.filter == EVFILT_WRITE)
	{
		std::cout << "SendMessage\n";
		size_t	trgt_pos = response.GetBody().find("Trgt=");
		if (trgt_pos != std::string::npos && request.GetMethod() == POST)
		{
			session->AddSessionData(response.GetBody().substr(trgt_pos + 5));
			std::cout << "auth data added: " << response.GetBody().substr(trgt_pos + 5) << '\n';
		}
		SendMessage();
		return ;
	}
	else
	{
		progress = END_CONNECTION;
		return ;
	}
}

void	Connection::ReadClient()
{
	char				buffer[1000000];
	memset(buffer, 0, 1000000);
	std::cout << client_socket_fd << "\n";
	ssize_t				nread = read(client_socket_fd, buffer, sizeof(buffer));

	if (nread > 0)
	{
		// hyunjunl 작품
		// EOF 말고는 실제로 출력이 안 됐음 ./webserv 1>/dev/null
		if (nread == 0)
			std::cerr << "hyunjunl " << client_socket_fd << std::endl;
		// 이걸로 소켓 끊어진 통신에 대해서 연결 해제하는 방식으로 구현하는 부분 rfc 문서에서 확인해서 적용해보기

		std::string &parse_data = parser.GetData();
		parse_data = parse_data + std::string(buffer, nread);
		std::cout << "\n\n원본 메시지 토탈\n" << parse_data;
		// for (ssize_t i = 0; i < nread; i++)
		// {
		// 	std::cout << "\n이번 메시지 끝문자 : " << (int)buffer[i];
		// }
		std::cout << "\n이번 메시지 시작문자 : " << (int)buffer[0];
		std::cout << "\n이번 메시지 끝문자 : " << (int)buffer[nread-1];
		std::cout << "\n데이터 길이 : " << nread << '\n';
		std::cout << "이번 읽기 대상\n";
		if (request.GetStatus() == READ_STARTLINE)
			std::cout << "READ_STARTLINE\n";
		if (request.GetStatus() == READ_HEADER)
			std::cout << "READ_HEADER\n";
		if (request.GetStatus() == READ_BODY)
			std::cout << "READ_BODY\n";
		if (request.GetStatus() == READ_TRAILER)
			std::cout << "READ_TRAILER\n";
		if (parse_data.find("\r\n") != std::string::npos)
			parser.ParseStartline(request);
		std::cout << request.GetMethod() << '\n';
		if (parse_data.find("\r\n\r\n") != std::string::npos)
		{
			parser.ParseHeader(request);
			// std::cout << "CRLFCRLF FOUND\n";
		}
		parser.ParseBody(request);
		parser.ParseTrailer(request);
		// std::cout << "TRAILER? status : " << request.GetStatus() << '\n';
		if (request.GetStatus() != READ_DONE && request.GetStatus() != BAD_REQUEST)
			progress = READ_CONTINUE;
		else
		{
			// std::cout << "\n총 파싱 데이터 len : " << request.GetBody().size() << '\n';
		// 	std::cout << "파싱 메시지\n";
		// 	std::cout << request.GetMethod() << " " << request.GetUrl() << " " << request.GetVersion() << '\n';
			// std::map<std::string, std::string> tmp_map = request.GetHeader();
			// for (std::map<std::string, std::string>::iterator it = tmp_map.begin(); it != tmp_map.end(); ++it)
			// 	std::cout << it->first << ": \'" << it->second << "\'\n";
			// std::cout << '\'' << request.GetBody() << "\'\n";

			// std::cout << "status = " << request.GetStatus() << '\n';
		}
		return ;
	}
	else
	{
		progress = END_CONNECTION;
		return ;
	}
}

void	Connection::MakeResponse()
{
	//request 유효성 검사
	if (request.GetStatus() == BAD_REQUEST)
	{
		response.make_response_40x(400);
		progress = COMBINE;
		return ;
	}
	//http/1.1 인데 host 헤더가 없을 때
	if (request.GetVersion() == "HTTP/1.1")
	{
		if (request.GetHeader().find("host") == request.GetHeader().end())
		{
			std::cout << "cant find host header\n";
			response.make_response_40x(400);
			progress = COMBINE;
			return ;
		}
	}
	//request에 맞는 server 찾기
	server_ptr = &config_ptr->GetServerVec().front();
	std::cout << server_ptr << "\n";
	for (size_t i = 0; i < config_ptr->GetServerVec().size(); i++)
	{
		if (request.GetHeader()["host"] == config_ptr->GetServerVec()[i].GetServerName())
		{
			server_ptr = &config_ptr->GetServerVec()[i];
			break ;
		}
	}

	//request에 맞는 location 찾기
	locate_ptr = NULL;
	const std::vector<Locate>& locate_vec = server_ptr->GetLocateVec();
	size_t	longest = 0;
	for (size_t i = 0; i < locate_vec.size(); ++i)
	{
		const std::string& s = request.GetUrl();
		std::cout << "url: " << request.GetUrl() << "\n";
		std::cout << "findurl: " << locate_vec[i].GetLocatePath() << "\n";
		if (s.find(locate_vec[i].GetLocatePath()) != std::string::npos)
		{
			if (locate_vec[i].GetLocatePath().size() > longest)
			{
				longest = locate_vec[i].GetLocatePath().size();
				locate_ptr = &locate_vec[i];
				std::cout << "longest: " << longest << "\n";
				std::cout << "url: " << request.GetUrl() << "\n";
			}
		}
	}
	if (locate_ptr == NULL)
	{
		std::cout << "cant find locate\n";
		response.make_response_40x(404);
		progress = COMBINE;
		return ;
	}
	//request 유효성 검사끝


	//method 종류를 지원하는지 확인
	if (request.GetMethod() == OTHER)
	{
		std::cout << "method not defined\n";
		response.make_response_40x(405);
		progress = COMBINE;
		return ;
	}

	//method 가 해당 로케이션에서 쓸 수 있는지 확인
	const std::vector<enum Method>& vec = locate_ptr->GetMethodVec();
	if (vec.size() != 0)
	{
		int flag = 0;
		for (size_t i = 0; i < vec.size(); i++)
		{
			if(vec[i] == request.GetMethod())
			{
				flag = 1;
				break ;
			}
		}
		if (flag == 0)
		{
			std::cout << " method: " << request.GetMethod() << "\n";
			for (size_t i = 0; i < locate_ptr->GetMethodVec().size(); i++)
				std::cout << " allowed method: " << locate_ptr->GetMethodVec()[i] << "\n";
			std::cout << "cant find allowd method\n";
			response.make_response_40x(405);
			progress = COMBINE;
			return ;
		}
	}

	//location에 리다이렉션 (return문)이 있는지 확인
	if (!locate_ptr->GetRedirectPair().second.empty())
	{
		int code = locate_ptr->GetRedirectPair().first;
		if (request.GetBody().size() > 100)
			response.make_response_40x(413);
		else
			response.make_response_30x(code, locate_ptr->GetRedirectPair().second);
		std::cout << "redirected with" << code << "\n";
		progress = COMBINE;
		return ;
	}

	//path 만들기
	path = "";
	path += locate_ptr->GetRoot();
	if (locate_ptr->GetLocatePath() == "/")
		path += "/";
	path += request.GetUrl().substr(locate_ptr->GetLocatePath().size());
	std::cout << "path: " << path << "\n";

	if (request.GetMethod() == GET)
		ProcessGet();
	else if (request.GetMethod() == POST)
		ProcessPost();
	else if (request.GetMethod() == DELETE)
		ProcessDelete();
}

void	Connection::ProcessGet()
{
	if (path.back() == '/')
	{
		ProcessDir();
		return ;
	}
	else
	{
		ProcessFile();
		return ;
	}
}

void	Connection::ProcessPost()
{
	std::cout << "Processpost\n";
	std::cout << "allowed body size" << locate_ptr->GetClientBodySize() << "\n";
	std::cout << "real body size" << request.GetBody().size() << "\n";
	if (request.GetBody().size() > static_cast<size_t>(locate_ptr->GetClientBodySize()))
	{
		response.make_response_40x(413);
		progress = COMBINE;
		return ;
	}
	std::vector<std::string> cgi = utils::SplitToVector(path, '.');
	std::string extension = cgi.back();
	std::cout << "extension: " << extension << "\n";
	if (locate_ptr->GetCgiMap().find(extension) != locate_ptr->GetCgiMap().end())
	{
		std::cout << locate_ptr->GetCgiMap().find(extension)->first << "\n";
		std::cout << locate_ptr->GetCgiMap().find(extension)->second << "\n";
		path = "";
		path = (locate_ptr->GetCgiMap()).find(extension)->second;
		ProcessCgi();
		return ;
	}
	else
	{
		response.SetStatus(200);
		progress = COMBINE;
	}
}

void	Connection::ProcessDelete()
{
	struct stat buf;
	if (stat(path.c_str(), &buf) == -1)
	{
		std::cout << "ProcessFile: open failed\n";
		response.make_response_40x(404);
		progress = COMBINE;
		return ;
	}
	if (session->CheckAuth(path.substr(path.rfind('/') + 1)) == true)
	{
		std::cout << "test : auth\n";
		std::remove(path.c_str());
		response.SetStatus(202);
	}
	else
	{
		std::cout << "test : no auth\n";
		response.make_response_40x(403);
	}
	progress = COMBINE;
	return ;
}


void	Connection::ProcessDir()
{
	//if method != GET
	std::cout << "ProcessDir\n";
	//if request as directory but it is file
	struct stat buf;
	if (stat(path.c_str(), &buf) == -1)
	{
		std::cout << "path: " << path << "\n";
		std::cout << "file not exist\n";
		response.make_response_40x(404);
		progress = COMBINE;
		return;
	}
	if ((buf.st_mode & R_OK) == 0)
	{
		std::cout << "file cant read\n";
		response.make_response_40x(403);
		progress = COMBINE;
		return ;
	}
	//check if default index file exist
	if (!locate_ptr->GetIndexVec().empty())
	{
		const std::vector<std::string> &index_vec = locate_ptr->GetIndexVec();
		std::cout << index_vec.size() << "\n";
		for (size_t i = 0; i < index_vec.size(); i++)
		{
			std::string temp = path;
			temp += index_vec[i];
			struct stat buf2;
			if (stat(temp.c_str(), &buf2) != -1)
			{
				std::cout << "indexpath: " << temp << "\n";
				path = temp;
				ProcessFile();
				return ;
			}
			std::cout << "cant stat indexpath: " << temp << "\n";
		}
	}
	//check if autoindex is on
	if (locate_ptr->GetAutoIndex())
	{
		response.AutoIndex(path);
		progress = COMBINE;
		return ;
	}
	else
	{
		response.make_response_40x(404);
		progress = COMBINE;
		return ;
	}
}

void	Connection::ProcessFile()
{
	std::cout << "ProcessFile\n";

	//check if file exist
	struct stat buf;
	if (stat(path.c_str(), &buf) == -1)
	{
		std::cout << "ProcessFile: open failed\n";
		response.make_response_40x(404);
		progress = COMBINE;
		return ;
	}
	//request as file but is directory
	if (S_ISDIR(buf.st_mode))
	{
		std::cout << "ProcessFile: is dir\n";
		response.make_response_30x(301, request.GetUrl() + "/");
		progress = COMBINE;
		return ;
	}
	file_fd = open(path.c_str(), std::ios::binary);
	std::cout << errno << "\n";
	if (file_fd == -1)
	{
		std::cout << "ProcessFile: open file failed\n";
		response.make_response_40x(403);
		progress = COMBINE;
		return ;
	}
	//파일 non-block 설정
	int flag = fcntl(file_fd, F_GETFL, 0);
	fcntl(file_fd, F_SETFL, flag | O_NONBLOCK);

	progress = FROM_FILE;
}

void	Connection::SendMessage()
{
	static int i = 0;
	const std::string &buffer = response.GetMessage();
	std::cout << "---------message-------------\n";
	// std::cout << response.GetMessage();
	std::cout << i++ << " " <<response.GetStatus() << "messagesize: " << buffer.size() << " to " << client_socket_fd <<"\n";
	std::cout << "---------message end-------------\n";
	ssize_t send_size = write(client_socket_fd, buffer.data(), buffer.size());
	std::cout << "send_size = " << send_size << "\n";
	if (send_size < 0)
	{
		std::cout << "read error\n";
		progress = END_CONNECTION;
		return ;
	}
	else if (send_size == response.GetMessageSize())
	{
		//response 완료
		progress = END_CONNECTION;
		return ;
	}
	else
	{
		//send_size만큼 message에서 지우기
		response.CutMessage(send_size);
		return ;
	}
}

void	Connection::ProcessCgi()
{
	std::cout << "ProcessCgi\n";
	//check if file exist
	struct stat tmp;
	std::cout << "path: " << path << "\n";
	if (stat(path.c_str(), &tmp) == -1)
	{
		std::cout << "cannot find path\n";
		response.make_response_40x(404);
		progress = COMBINE;
		return ;
	}
	//환경변수 설정
	cgi.setEnv(request, *server_ptr);
	std::cout << "setenv setup done\n";

	//pipe 설정
	cgi.setPipe();
	std::cout << "pipe setup done\n";

	//cgi 실행
	if (cgi.CgiExec(path).ok())
	{
		pipein = cgi.GetPipeIn();
		pipeout = cgi.GetPipeOut();
		std::cout << "cgi setup done\n";
		utils::AddReadEvent(kq, pipein);
		std::cout << "AddReadEvent4 fd: " << pipein << '\n';
		utils::AddWriteEvent(kq, pipeout);
		std::cout << "AddWriteEvent1 fd: " << pipeout << '\n';
		response.BodyResize(request.GetBody().size() + 700);
		progress = CGI;
	}
	else
	{
		response.make_response_50x(500);
		progress = COMBINE;
	}
}

void	Connection::ReadCgi()
{
	std::string buff;
	ssize_t	maxsize = 65536;
	buff.resize(maxsize);
	ssize_t readsize = read(pipein, &buff[0], maxsize);
	if (readsize < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return ;
		std::cout << "readcgi error\n";
		std::cout << errno << "\n";
		response.make_response_50x(500);
		progress = COMBINE;
		return ;
	}
	else if (readsize == 0)
	{
		std::cout << "readcgi done << "<<readsize<<"\n";
		response.AddBody(buff, readsize);
		response.SplitBodyHeaderData();
		utils::RemoveReadEvent(kq, pipein);
		std::cout << "RemoveReadEvent3 fd: " << pipein << '\n';
		progress = COMBINE;
		return ;
	}
	else
	{
		std::cout << "readcgi again << "<<readsize<<"\n";
		response.AddBody(buff, readsize);
		return ;
	}
}

void	Connection::SendCgi()
{
	std::cout << "SendCgi\n";
	ssize_t writesize = write(pipeout, request.GetBody().data(), request.GetBody().size());
	std::cout << "\nsize!@!@" << writesize << '\n';
	if (writesize < 0)
	{
		std::cout << "send cgi error\n";
		std::cout << errno << "\n";
		response.make_response_50x(500);
		progress = COMBINE;
		return ;
	}
	else if (!request.GetBody().empty())
	{
		std::cout << "send cgi again: write success this time by "<<writesize<<"\n";
		request.CutBody(writesize);
		std::cout << "left in reqestbody: " << request.GetBody().size() << "\n";
		return ;
	}
	else
	{
		std::cout << "send cgi done: write success this time by "<<writesize<<"\n";
		request.CutBody(writesize);
		utils::RemoveWriteEvent(kq, pipeout);
		std::cout << "RemoveWriteEvent3 fd: " << pipeout << '\n';
		close(pipeout);
		return ;
	}
}

void	Connection::ReadFile()
{
	static char buff[150000000];
	ssize_t readsize = read(file_fd, &buff[0], 150000000);
	std::cout << "readfile: " << readsize << "\n";
	if (readsize < 0)
	{
		response.make_response_50x(500);
		progress = COMBINE;
		return ;
	}
	else if (readsize == 150000000)
	{
		response.AddBody(buff, readsize);
		return ;
	}
	else
	{
		response.AddBody(buff, readsize);
		progress = COMBINE;
		utils::RemoveReadEvent(kq, file_fd);
		std::cout << readsize << ": ReadFile done\n";
		return ;
	}
}

void	Connection::CheckExitCgi()
{
	int	status;

	if (cgi.GetPid() != 0)
	{
		waitpid(cgi.GetPid(), &status, WNOHANG);
		if (!WIFEXITED(status))
		{
			if (kill(cgi.GetPid(), SIGINT) == -1)
			{
				std::cout << "-------------kill failed------------------";
				// response.make_response_50x(500);
				// progress = COMBINE;
			}
		}
		// std::cout << "-------------killed child------------------";
		cgi.SetPid(0);
	}
}

int		Connection::GetClientSocketFd()
{
	return client_socket_fd;
}

enum CurrentProgress		Connection::GetProgress()
{
	return progress;
}

void		Connection::SetProgress(enum CurrentProgress progress)
{
	this->progress = progress;
}

int		Connection::GetFileFd()
{
	return file_fd;
}
int		Connection::GetPipein()
{
	return pipein;
}

int		Connection::GetPipeout()
{
	return pipeout;
}

std::time_t		Connection::GetTimeval()
{
	return timeval;
}

Request&	Connection::GetRequest()
{
	return request;
}

Response&	Connection::GetResponse()
{
	return response;
}

int		Connection::GetKq()
{
	return kq;
}

void	Connection::UpdateTimeval()
{
	std::time(&timeval);
}

void	Connection::SetPipein(int fd){
	pipein = fd;
}

void	Connection::SetPipeout(int fd){
	pipeout = fd;
}

void	Connection::SetFileFd(int fd){
	file_fd = fd;
}

Cgi&	Connection::GetCgi()
{
	return (cgi);
}

Parser&	Connection::GetParser()
{
	return (parser);
}

void	Connection::Cleaner()
{
	progress = FROM_CLIENT;
	file_fd = 0;
	pipein = 0;
	pipeout = 0;
	total_len = 0;
}
