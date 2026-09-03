#pragma once

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <cctype>
#include <algorithm>
#include <limits>
#include <variant>
#include <unordered_map>
#include <array>
#include <queue>
#include <typeinfo>
#include <cstdio>
#include <thread>
#include <chrono>
#include <curl/curl.h>



#include "net/protocol/messenger.hpp"
#include "net/https/httpCodec.hpp"
#include "net/routing/router.hpp"
#include "net/transport/socket_compat.hpp"
#include "net/websocket/webSocket.hpp"
#include "net/transport/session.hpp"

#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#pragma comment(lib, "Ws2_32.lib")


#else
	#include <cerrno>
	#include <netdb.h>
	#include <sys/socket.h>
	#include <unistd.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>

#endif


namespace anime::net {
using namespace anime::protocol;
using namespace anime::app;
using namespace anime::net::http;
using namespace anime::net::websocket;
const uint16_t DEFAULT_BUFLEN = 512;

const uint16_t SERVER_PORT = 443;

const uint16_t INTERNAL_HTTP_PORT = 8081;
const uint16_t INTERNAL_WS_PORT = 8080;

struct Request {
	SocketHandle client;
	AppRequest request;
};

struct Response {
	SocketHandle client;
	AppResponse response;
};

struct WebSOCKFrame {
	bool fin;
	uint8_t opcode;
	std::vector<uint8_t> payload;
};

class NetService {
	public:
		explicit NetService(Context& context);

		void start();

		void run();

		void stop();
	private:
		Context& context;
		std::atomic_bool running = false;
		std::vector<std::thread> listenerThreads;



		void tcpServer(uint16_t socket, void (NetService::*handler)(SocketHandle));

		void websocketHandler(SocketHandle ClientSocket);

		void httpHandler(SocketHandle ClientSocket);
};

} // namespace anime::net
