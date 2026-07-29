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


#include "../protocol/messenger.hpp"
#include "../routing/router.hpp"
#include "socket_compat.hpp"
#include "../websocket/webSocket.hpp"
#include "session.hpp"

#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#pragma comment(lib, "Ws2_32.lib")
	#include <http.h>
	#include <winhttp.h>

#else
	#include <cerrno>
	#include <netdb.h>
	#include <sys/socket.h>
	#include <unistd.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>

#endif


#define DEFAULT_BUFLEN 512

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

		void stop();
	private:
		Context& context;
	    std::atomic_bool running = false;

		void clientHandler(SocketHandle ClientSocket);

		void tcpServer(uint16_t socket, void (NetService::*handler)(SocketHandle));

		void websocketHandler(SocketHandle ClientSocket);

		void httpHandler(SocketHandle ClientSocket);
};
