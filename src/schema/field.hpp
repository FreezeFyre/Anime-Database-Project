#pragma once

#include <optional>
#include <string>

#include "io/file_types.hpp"

namespace anime::schema {

enum class FIELD_PRESENCE { REQUIRED, RECOMMENDED, FIELD_OPTIONAL };
enum class VALUE_REFERENCE_TYPE { NONE, BOUNDS, LOOKUP_TABLE, FOREIGN_RECORD, ENTRY_FIELD };
enum class UPDATE_OPERATION : std::uint8_t { SET_FIELD, CLEAR_FIELD, ADD_VALUE, REMOVE_VALUE };

struct Bound { std::uint64_t lower; std::uint64_t upper; };

struct FieldInfo {
	std::string name;
	anime::storage::schemaField::dataType type;
	anime::storage::schemaField::filterMode mode = anime::storage::schemaField::filterMode::FM_NONE;
	FIELD_PRESENCE presence = FIELD_PRESENCE::FIELD_OPTIONAL;
	VALUE_REFERENCE_TYPE reference = VALUE_REFERENCE_TYPE::NONE;
	std::optional<Bound> bounds = std::nullopt;
	std::string tableTitle;
};

} // namespace anime::schema
