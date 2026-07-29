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




// Temporary context stuff, move to app
#include "../../core.hpp"
#include "../../databases/entry_data.hpp"
#include "../../databases/users.hpp"
#include "../../databases/auth/auth_service.hpp"
#include "../../filtering/filterProcessor.hpp"
#include "../protocol/data.hpp"
#include "../transport/session.hpp"

struct Context {
	entryDB entries;
	filterProcessor filterService;
	userDB users;

	mutable std::shared_mutex entriesMutex;
	mutable std::shared_mutex usersMutex;
};

using AppPayload = std::variant<
	EntryRequest
>;

using ResponsePayload = std::variant<
	EntryResponse
>;

struct AppRequest {
	COM_TYPE type;
	DOMAIN_TYPE domain;
	ACTION_TYPE action;
	uint32_t id;
	AppPayload payload;
};

struct AppResponse {
	uint32_t id;
	ResponsePayload payload;
	bool closeConnection = false;
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

class Router {
    public:
	    AppResponse route(AppRequest& request, Context& context, Session& session);

	private:
		AppResponse handle(const EntryRequest& request, uint32_t id, Context& context, Session& session);

};
