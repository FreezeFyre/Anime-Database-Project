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

struct FieldInfo {
	std::string name;
	schemaField::dataType type;
	schemaField::filterMode mode;
	fieldPresence presence;

	ValueRef reference = ValueRef::NONE;
	std::optional<Bound> bounds;
	std::string tableTitle;
}; // May want to add field default values to this
