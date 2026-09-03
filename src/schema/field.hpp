#pragma once

#include <optional>
#include <string>

#include "io/file_types.hpp"

namespace anime::schema {

enum class FIELD_PRESENCE { REQUIRED, RECOMMENDED, FIELD_OPTIONAL };
enum class VALUE_REFERENCE_TYPE { NONE, BOUNDS, LOOKUP_TABLE, FOREIGN_RECORD };
enum class UPDATE_OPERATION : std::uint8_t { SET_FIELD, CLEAR_FIELD, ADD_VALUE, REMOVE_VALUE };

struct Bound { int lower; int upper; };

struct FieldInfo {
	std::string name;
	anime::storage::schemaField::dataType type;
	anime::storage::schemaField::filterMode mode;
	FIELD_PRESENCE presence;
	VALUE_REFERENCE_TYPE reference = VALUE_REFERENCE_TYPE::NONE;
	std::optional<Bound> bounds;
	std::string tableTitle;
};

} // namespace anime::schema
