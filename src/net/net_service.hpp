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
#include "socket_compat.hpp"

#define DEFAULT_BUFLEN 512

struct Request {
    SocketHandle client;
    Message message;
};

struct Response {
    SocketHandle client;
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

		void clientHandler(SocketHandle ClientSocket);

};

