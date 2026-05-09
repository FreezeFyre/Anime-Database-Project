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

#include "messenger.hpp"

#pragma once


/*

[ENCODE_VERSION]
[COM_TYPE]
[DOMAIN]
[ACTION]
[ID]
[PAYLOAD_SIZE]
[PAYLOAD]

*/
struct RouteResult {
	Message message;
	bool closeConnection = false;
};

class Router {
	public:
		RouteResult route(Message& message);

	private:
};