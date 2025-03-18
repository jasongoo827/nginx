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
#include <algorithm>

Connection::Connection(int kq, int clinet_socket_fd, sockaddr_in client_socket_addr, Config* config_ptr, Session* session)
: request(), response(), cgi()
{
	this->client_socket_fd = clinet_socket_fd;
	this->client_socket_addr = client_socket_addr;
	this->config_ptr = config_ptr;
	this->server_ptr = NULL;
	this->locate_ptr = NULL;
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
	this->server_ptr = ref.server_ptr;
	this->locate_ptr = ref.locate_ptr;
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
	this->server_ptr = ref.server_ptr;
	this->locate_ptr = ref.locate_ptr;
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
		response.AddHeader("Connection", "keep-alive");
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
	char				buffer[1300000];
	ssize_t				nread = read(client_socket_fd, buffer, sizeof(buffer));

	if (nread > 0)
	{
		std::string &parse_data = parser.GetData();
		parse_data = parse_data + std::string(buffer, nread);
		// std::cout << "\n\n원본 메시지 토탈\n" << parse_data;
		// for (ssize_t i = 0; i < nread; i++)
		// {
		// 	std::cout << "이번 메시지 : " << (int)buffer[i];
		// }
		if (parse_data.find("\r\n") != std::string::npos)
			parser.ParseStartline(request);
		if (parse_data.find("\r\n\r\n") != std::string::npos)
		{
			parser.ParseHeader(request);
			if (server_ptr == NULL)
				SetServerData();
			if (locate_ptr == NULL)
				SetLocateData();
			parser.SetMaxBodySize(locate_ptr->GetClientBodySize());
			request.ReserveBody(locate_ptr->GetClientBodySize());
		}
		parser.ParseBody(request);
		parser.ParseTrailer(request);
		if (request.GetStatus() != READ_DONE && request.GetStatus() != BAD_REQUEST)
			progress = READ_CONTINUE;
		else if (request.GetStatus() == BAD_REQUEST)
			request.SetStatus(READ_DONE);
	}
	else
		progress = END_CONNECTION;
}

void	Connection::MakeResponse()
{
	//request 유효성 검사
	if (locate_ptr != NULL)
		response.BodyResize(locate_ptr->GetClientBodySize());
	else
	{
		MakeDefaultErrorPage(400);
		progress = COMBINE;
		return ;
	}
	if (request.GetStatus() == BAD_REQUEST)
	{
		MakeErrorPage(400);
		return ;
	}
	//http/1.1 인데 host 헤더가 없을 때
	if (request.GetVersion() == "HTTP/1.1")
	{
		if (request.GetHeader().find("host") == request.GetHeader().end())
		{
			std::cout << "cant find host header\n";
			MakeErrorPage(400);
			return ;
		}
	}

	if (locate_ptr == NULL)
	{
		std::cout << "cant find locate\n";
		MakeErrorPage(404);
		return ;
	}
	//request 유효성 검사끝


	//method 종류를 지원하는지 확인
	if (request.GetMethod() == OTHER)
	{
		MakeErrorPage(405);
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
			MakeErrorPage(405);
			return ;
		}
	}

	//location에 리다이렉션 (return문)이 있는지 확인
	if (!locate_ptr->GetRedirectPair().second.empty())
	{
		int code = locate_ptr->GetRedirectPair().first;
		response.make_response_30x(code, locate_ptr->GetRedirectPair().second);
		progress = COMBINE;
		return ;
	}

	//path 만들기
	path = "";
	path += locate_ptr->GetRoot();
	if (locate_ptr->GetLocatePath() == "/")
		path += "/";
	path += request.GetUrl().substr(locate_ptr->GetLocatePath().size());

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
	if (request.GetBody().size() > static_cast<size_t>(locate_ptr->GetClientBodySize()))
	{
		MakeErrorPage(413);
		return ;
	}
	std::vector<std::string> cgi = utils::SplitToVector(path, '.');
	std::string extension = cgi.back();
	if (locate_ptr->GetCgiMap().find(extension) != locate_ptr->GetCgiMap().end())
	{
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
		std::cout << "ProcessDelete: file not exist\n";
		response.make_response_40x(404);
		progress = COMBINE;
		return ;
	}
	if (session->CheckAuth(path.substr(path.rfind('/') + 1)) == true)
	{
		if (std::remove(path.c_str()) == -1)
		{
			MakeErrorPage(500);
			session->AddSessionData(path.substr(path.rfind('/') + 1));
			return ;
		}
		response.SetStatus(202);
		progress = COMBINE;
	}
	else
	{
		MakeErrorPage(403);
		return ;
	}
}


void	Connection::ProcessDir()
{
	//if method != GET
	//if request as directory but it is file
	struct stat buf;
	if (stat(path.c_str(), &buf) == -1)
	{
		std::cout << "file not exist\n";
		MakeErrorPage(404);
		return;
	}
	if ((buf.st_mode & R_OK) == 0)
	{
		std::cout << "file cant read\n";
		MakeErrorPage(403);
		return ;
	}
	//check if default index file exist
	if (!locate_ptr->GetIndexVec().empty())
	{
		const std::vector<std::string> &index_vec = locate_ptr->GetIndexVec();
		for (size_t i = 0; i < index_vec.size(); i++)
		{
			std::string temp = path;
			temp += index_vec[i];
			struct stat buf2;
			if (stat(temp.c_str(), &buf2) != -1)
			{
				path = temp;
				ProcessFile();
				return ;
			}
			if (!locate_ptr->GetAutoIndex())
			{
				MakeErrorPage(404);
				return ;
			}
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
		MakeErrorPage(403);
		return ;
	}
}

void	Connection::ProcessFile()
{
	//check if file exist
	struct stat buf;
	if (stat(path.c_str(), &buf) == -1)
	{
		MakeErrorPage(404);
		return ;
	}
	//request as file but is directory
	if (S_ISDIR(buf.st_mode))
	{
		response.make_response_30x(301, request.GetUrl() + "/");
		progress = COMBINE;
		return ;
	}
	if (buf.st_mode & S_IRUSR)
	{
		file_fd = open(path.c_str(), std::ios::binary);
		if (file_fd == -1)
		{
			std::cout << "ProcessFile: open file failed\n";
			MakeErrorPage(500);
			return ;
		}
		//파일 non-block 설정
		if (utils::SetNonBlock(file_fd) == false)
		{
			MakeErrorPage(500);
			return ;
		}
		progress = FROM_FILE;
	}
	else
	{
		MakeErrorPage(403);
		return ;
	}
}

void	Connection::SendMessage()
{
	const std::string &buffer = response.GetMessage();
	ssize_t send_size = write(client_socket_fd, buffer.data() + response.GetMessagePos(), buffer.size() - response.GetMessagePos());
	if (send_size < 0)
	{
		std::cout << "read error\n";
		progress = END_CONNECTION;
		return ;
	}
	else if (send_size == 0)
	{
		//response 완료
		progress = END_CONNECTION;
		return ;
	}
	else
	{
		response.AddMessagePos(send_size);
		return ;
	}
}

void	Connection::ProcessCgi()
{
	//check if file exist
	struct stat tmp;
	if (stat(path.c_str(), &tmp) == -1)
	{
		std::cout << "cannot find path\n";
		MakeErrorPage(404);
		return ;
	}
	//환경변수 설정
	cgi.setEnv(request, *server_ptr);

	//pipe 설정
	cgi.setPipe();

	//cgi 실행
	if (cgi.CgiExec(path).ok())
	{
		pipein = cgi.GetPipeIn();
		pipeout = cgi.GetPipeOut();
		utils::AddReadEvent(kq, pipein);
		utils::AddWriteEvent(kq, pipeout);
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
	static char buff[1000000];
	ssize_t readsize = read(pipein, buff, 1000000);
	if (readsize < 0)
	{
		std::cout << "readcgi error\n";
		response.make_response_50x(500);
		progress = COMBINE;
		return ;
	}
	else if (readsize == 0)
	{
		response.AddBody(buff, readsize);
		response.SplitBodyHeaderData();
		utils::RemoveReadEvent(kq, pipein);
		size_t trgt_pos = response.GetBody().find("Trgt=");
		if (trgt_pos != std::string::npos && request.GetMethod() == POST)
			session->AddSessionData(response.GetBody().substr(trgt_pos + 5));
		progress = COMBINE;
		return ;
	}
	else
	{
		response.AddBody(buff, readsize);
		return ;
	}
}

void	Connection::SendCgi()
{
	ssize_t writesize = write(pipeout, request.GetBody().data() + request.GetBodyPos(), request.GetBody().size() - request.GetBodyPos());
	if (writesize < 0)
	{
		std::cout << "send cgi error\n";
		response.make_response_50x(500);
		progress = COMBINE;
		return ;
	}
	else if (writesize == 0)
	{
		request.AddBodyPos(writesize);
		utils::RemoveWriteEvent(kq, pipeout);
		close(pipeout);
		return ;
	}
	else
	{
		request.AddBodyPos(writesize);
		return ;
	}
}

void	Connection::ReadFile()
{
	static char buff[110000000];
	ssize_t readsize = read(file_fd, &buff[0], 110000000);
	if (readsize < 0)
	{
		response.make_response_50x(500);
		progress = COMBINE;
		return ;
	}
	else if (readsize == 110000000)
	{
		response.AddBody(buff, readsize);
		return ;
	}
	else
	{
		response.AddBody(buff, readsize);
		progress = COMBINE;
		utils::RemoveReadEvent(kq, file_fd);
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
				std::cout << "kill failed\n";
				MakeErrorPage(500);
			}
		}
		cgi.SetPid(0);
	}
}

void   Connection::MakeErrorPage(int status)
{
	response.SetStatus(status);
	std::string errorpage = "";
	if (server_ptr->GetErrorPage().find(status) != server_ptr->GetErrorPage().end())
	{
		errorpage = server_ptr->GetErrorPage().find(status)->second;
		std::cout << errorpage << "\n";
	}
	if (errorpage == "")
	{
		MakeDefaultErrorPage(status);
		progress = COMBINE;
		return ;
	}
	else
	{
		struct stat buf;
		if (stat(errorpage.c_str(), &buf) == -1)
				MakeDefaultErrorPage(status);
		if (file_fd != 0)
				close(file_fd);
		file_fd = open(errorpage.c_str(), std::ios::binary);
		if (file_fd == -1)
		{
				std::cout << "ProcessErr: open file failed\n";
				MakeDefaultErrorPage(500);
				return ;
		}
		int flag = fcntl(file_fd, F_GETFL, 0);
		fcntl(file_fd, F_SETFL, flag | O_NONBLOCK);

		progress = FROM_FILE;
	}
}

void	Connection::MakeDefaultErrorPage(int status)
{
	response.SetStatus(status);
	response.MakeErrorBody(status);
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
	server_ptr = NULL;
	locate_ptr = NULL;
}

void   Connection::SetServerData()
{
	//request에 맞는 server 찾기
	server_ptr = &config_ptr->GetServerVec().front();
	for (size_t i = 0; i < config_ptr->GetServerVec().size(); i++)
	{
		if (request.GetHeader()["host"] == config_ptr->GetServerVec()[i].GetServerName())
		{
			server_ptr = &config_ptr->GetServerVec()[i];
			break ;
		}
	}
	if (server_ptr == NULL)
		request.SetStatus(BAD_REQUEST);
}

void   Connection::SetLocateData()
{
	//request에 맞는 location 찾기
	locate_ptr = NULL;
	const std::vector<Locate>& locate_vec = server_ptr->GetLocateVec();
	size_t  longest = 0;
	for (size_t i = 0; i < locate_vec.size(); ++i)
	{
		const std::string& s = request.GetUrl();
		if (s.find(locate_vec[i].GetLocatePath()) != std::string::npos)
		{
			if (locate_vec[i].GetLocatePath().size() > longest)
			{
				longest = locate_vec[i].GetLocatePath().size();
				locate_ptr = &locate_vec[i];
			}
		}
	}
}