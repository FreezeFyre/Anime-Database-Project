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
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma once
#include "messenger.hpp"
#include "router.hpp"

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
		MessageCodec codec;
		bool running = false;

		void clientHandler(SOCKET ClientSocket);

};
