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



// Temporary context stuff, move to app
#include "../databases/entry_data.hpp"
#include "../filtering/filterProcessor.hpp"
#include "data.hpp"

struct Context {
	entryDB entries;
	filterProcessor fltrSrvc;
};
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
		RouteResult route(Message& message, Context& context);

	private:
};
