# Webserv
HTTP Server in C++ 98 made by 42Seoul Webserv team "nginx".

![webserv gif](https://jasongoo827.github.io/assets/webserv.gif)

[English](#english) | [한국어](#korean)

## English

### Overview

A fully functional HTTP/1.1 server implementation in C++98, featuring non-blocking I/O, CGI support, and multiple port listening capabilities. Built from scratch without external libraries, this server handles concurrent connections efficiently using kqueue-based event-driven architecture.

### Key Features

- **HTTP/1.1 Compliant**: Full support for GET, POST, DELETE methods
- **Non-blocking I/O**: Event-driven architecture using kqueue/poll
- **CGI Support**: Dynamic content generation with PHP/Python
- **Multiple Ports**: Simultaneous listening on multiple ports
- **File Upload**: Support for large file uploads (100MB+)
- **Virtual Hosts**: Multiple server configurations
- **Error Handling**: Custom error pages and robust error management

### Technical Achievements

- **Performance**: Handles 1000+ concurrent connections
- **Throughput**: Processes 5000+ requests/second
- **Stability**: Zero memory leaks (Valgrind verified)
- **Stress Testing**: Maintains availability under heavy load

### Architecture

- **Event-Driven Model**: Single-threaded with kqueue for I/O multiplexing
- **Configuration Parser**: NGINX-like configuration file support
- **Request Pipeline**: Efficient HTTP request parsing and response generation
- **Resource Management**: Smart pointer-like RAII for file descriptors

### Build and Run

```bash
# Clone and build
git clone https://github.com/jasongoo827/nginx.git
cd nginx
make

# Run with configuration
./webserv [config_file]

# Default configuration is used if no argument provided
./webserv
```

### Contributors

- **jgoo**: Configuration file parser, System architecture
- **dongyeuk**: HTTP request parsing, Method handling
- **yakim**: HTTP response generation, CGI implementation

### Documentation

- [Web Server Architecture](https://jasongoo827.github.io/blog/webserv)
- [I/O Multiplexing Implementation](https://jasongoo827.github.io/blog/IOMultiplexing)
- [HTTP Protocol & Configuration](https://jasongoo827.github.io/blog/HttpMessage)

---

## Korean

## 1. Requirements

- 서버는 설정 파일을 인자로 받아 실행되며, 인자가 없으면 기본 경로를 사용합니다.
- 외부 웹서버를 `execve`로 실행하는 것은 금지입니다.
- 서버는 블로킹 없이 동작해야 하며, 클라이언트가 적절히 종료될 수 있어야 합니다.
- **모든 I/O는 단 하나의 `poll()` (또는 유사 함수) 호출로 처리해야 하며**, listen, read, write 모두 포함됩니다.
- `poll()`을 통하지 않은 read/write는 금지입니다. (`errno` 확인도 금지)
- 설정 파일은 `poll()` 없이 읽어도 됩니다.
- 파일 디스크립터는 논블로킹 모드여야 하며, 필요 시 `fcntl()`은 `O_NONBLOCK`, `FD_CLOEXEC`, `F_SETFL`만 허용됩니다 (macOS 한정).
- 요청은 절대로 영원히 대기 상태로 남아서는 안 됩니다.
- 브라우저와 호환되는 응답을 제공해야 하며, `NGINX`를 참조용으로 사용할 수 있습니다.
- HTTP/1.1 스펙을 준수해야 하며, 응답 상태 코드는 정확해야 합니다.
- 기본 오류 페이지를 제공해야 합니다 (사용자가 제공하지 않은 경우).
- CGI는 PHP, Python 등 확장자 기반으로 실행하며, `fork`는 CGI 용도 외에는 금지입니다.
- 정적 웹사이트를 서빙할 수 있어야 하며, 클라이언트의 파일 업로드도 지원해야 합니다.
- **최소한 GET, POST, DELETE** 메서드를 지원해야 합니다.
- 서버는 스트레스 테스트 상황에서도 가능한 한 가용 상태를 유지해야 합니다.
- **여러 포트를 동시에 리스닝** 할 수 있어야 합니다.

### Configuration File

- 여러 서버 블록을 설정 가능 (`server` 단위)
- 포트와 호스트, `server_name`, 기본 오류 페이지, 요청 본문 크기 제한 등 설정 가능
- 각 라우트(`location`)는 다음 중 하나 이상을 설정할 수 있어야 합니다:
  - 허용 메서드 지정 (GET, POST, DELETE)
  - 리다이렉션
  - 정적 파일 또는 디렉토리 루트 설정
  - 디렉토리 리스팅 on/off
  - 디폴트 파일 설정 (예: `index.html`)
  - 파일 확장자에 따른 CGI 실행
  - 파일 업로드 허용 및 저장 경로 지정
- **CGI 작동 조건**
  - `PATH_INFO`로 전체 경로 전달
  - 청크 요청은 서버가 언청크 처리 후 CGI에 전달
  - CGI 출력에서 `Content-Length`가 없다면 EOF로 종료 인식
  - CGI는 실행 디렉토리에서 작동해야 상대 경로가 올바르게 처리됨

## 2. Running the Project
``` shell
git clone https://github.com/jasongoo827/nginx.git

make

./webserv [configuration file]

```

## 3. Contributors
- jgoo: configuration file 
- dongyeuk: request
- yakim: response

## 4. Documentation
- [웹 서버란?](https://jasongoo827.github.io/blog/webserv)
- [I/O Multiplexing](https://jasongoo827.github.io/blog/IOMultiplexing)
- [Configuration File & Http Message](https://jasongoo827.github.io/blog/HttpMessage)
