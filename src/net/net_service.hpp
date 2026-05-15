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
#include "messenger.hpp"
#include "router.hpp"

#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>

#else
	#include <cerrno>
	#include <netdb.h>
	#include <sys/socket.h>
	#include <unistd.h>


#endif


#define DEFAULT_BUFLEN 512

struct Request {
    SOCKET client;
    Message message;
};

struct Response {
    SOCKET client;
    Message message;
};


class NetService {
	public:
		void start(uint16_t port);

		void stop();

	private:
		Router router;
		Context context;
		MessageCodec codec;
		bool running = false;

		void clientHandler(SOCKET ClientSocket);

};

