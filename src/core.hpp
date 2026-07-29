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

uint32_t ENCODE_VERSION = 1001;

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
	ACT_LOGOUT = 10
};
enum class DOMAIN_TYPE : uint8_t {
	ENTRY = 1,
	GROUPS = 2,
	CHARACTERS = 3,
	FILTERS = 4,
	USERS = 9,
	AUTH = 10
};

enum class UpdateOp : uint8_t {
	SET_FIELD,
	CLEAR_FIELD,
	ADD_VALUE,
	REMOVE_VALUE
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
