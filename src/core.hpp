#pragma once

// this file will eventuially be used to contain unified enums and such like value ref, field presence, and filter mode
// for use in the database and filtering systems also things like the FieldInfo struct and the FieldFilter struct will
// be moved here as well

#include <optional>
#include <string>

#include "io/fileHandling.hpp"

using UserId = uint32_t;
using IdentityId = uint32_t;
using SessionId = uint32_t;

enum class UserRole : uint8_t {
	VIEWER = 0,
	EDITOR = 1,
	ADMIN = 2
};

enum class fieldPresence {
	REQUIRED,
	RECOMMENDED,
	FIELD_OPTIONAL
};

enum class ValueRef {
	NONE,
	BOUNDS,
	LOOKUP_TABLE,
	FOREIGN_RECORD
};

struct Bound {
	int lower;
	int upper;
};

enum class LogLevel {
	Error,
	Warn,
	Info,
	Debug,
	Trace
};

inline constexpr uint32_t ENCODE_VERSION = 1001;

enum class COM_TYPE : uint8_t {
	REQ = 1,
	RESP = 2,
	STATUS = 3
};
enum class ACTION_TYPE : uint8_t {
	ACT_GET = 1,
	ACT_UPDATE = 2,
	ACT_CREATE = 3,
	ACT_DELETE = 4,
	ACT_SEARCH = 5,
	ACT_SORT = 6,
	ACT_APPLY = 7,
	ACT_FILTER = 8,
	ACT_LOGIN = 9,
	ACT_OAUTH = 10,
	ACT_LOGOUT = 11,
	ACT_UNKNOWN = 0
};
inline ACTION_TYPE action_type(std::string type) {
	if (type == "get") {
		return ACTION_TYPE::ACT_GET;
	} else if (type == "update") {
		return ACTION_TYPE::ACT_UPDATE;
	} else if (type == "create") {
		return ACTION_TYPE::ACT_CREATE;
	} else if (type == "delete") {
		return ACTION_TYPE::ACT_DELETE;
	} else if (type == "search") {
		return ACTION_TYPE::ACT_SEARCH;
	} else if (type == "sort") {
		return ACTION_TYPE::ACT_SORT;
	} else if (type == "apply") {
		return ACTION_TYPE::ACT_APPLY;
	} else if (type == "filter") {
		return ACTION_TYPE::ACT_FILTER;
	} else if (type == "login") {
		return ACTION_TYPE::ACT_LOGIN;
	} else if (type == "oauth") {
		return ACTION_TYPE::ACT_OAUTH;
	} else if (type == "logout") {
		return ACTION_TYPE::ACT_LOGOUT;
	}
	return ACTION_TYPE::ACT_UNKNOWN;
}

enum class DOMAIN_TYPE : uint8_t {
	ENTRY = 1,
	GROUPS = 2,
	CHARACTERS = 3,
	FILTERS = 4,
	USERS = 9,
	AUTH = 10,
	UNKNOWN = 0
};
inline DOMAIN_TYPE domain_type(std::string type) {
	if (type == "entry") {
		return DOMAIN_TYPE::ENTRY;
	} else if (type == "groups") {
		return DOMAIN_TYPE::GROUPS;
	} else if (type == "characters") {
		return DOMAIN_TYPE::CHARACTERS;
	} else if (type == "filters") {
		return DOMAIN_TYPE::FILTERS;
	} else if (type == "users") {
		return DOMAIN_TYPE::USERS;
	} else if (type == "auth") {
		return DOMAIN_TYPE::AUTH;
	}
	return DOMAIN_TYPE::UNKNOWN;
}

enum class UpdateOp : uint8_t {
	SET_FIELD,
	CLEAR_FIELD,
	ADD_VALUE,
	REMOVE_VALUE
};

struct HttpResponse {
	std::string version = "HTTP/1.1";
	uint16_t statusCode = 200;
	std::string reason = "OK";

	std::unordered_map<std::string, std::string> headers;
	ByteVector body;
};

struct Update {
	uint32_t id;
	UpdateOp operation;
	uint8_t fieldIndex;
	dataType type;
	std::variant<
		std::monostate,
		uint8_t,
		uint16_t,
		uint32_t,
		std::string,
		std::vector<int>
	> value;
};


struct FieldInfo {
	std::string name;
	schemaField::dataType type;
	schemaField::filterMode mode;
	fieldPresence presence;

	ValueRef reference = ValueRef::NONE;
	std::optional<Bound> bounds;
	std::string tableTitle;
}; // May want to add field default values to this
