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

#include <shared_mutex>

#include "../protocol/messenger.hpp"



// Temporary context stuff, move to app
#include "../../databases/entry_data.hpp"
#include "../../filtering/filterProcessor.hpp"
#include "../../core.hpp"
#include "../../databases/users.hpp"
#include "../protocol/data.hpp"
#include "../../databases/auth/auth_service.hpp"
#include "../transport/session.hpp"

struct Context {
	entryDB entries;
	filterProcessor filterService;
	userDB users;

	mutable std::shared_mutex entriesMutex;
	mutable std::shared_mutex usersMutex;
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
		RouteResult route(Message& message, Context& context, Session& session);

	private:
};
