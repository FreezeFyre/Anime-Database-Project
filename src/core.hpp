#pragma once

// this file will eventuially be used to contain unified enums and such like value ref, field presence, and filter mode
// for use in the database and filtering systems also things like the FieldInfo struct and the FieldFilter struct will
// be moved here as well

#include <optional>
#include <string>

#include "io/fileHandling.hpp"

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

struct FieldInfo {
	std::string name;
	schemaField::dataType type;
	schemaField::filterMode mode;
	fieldPresence presence;

	ValueRef reference = ValueRef::NONE;
	std::optional<Bound> bounds;
	std::string tableTitle;
}; // May want to add field default values to this
