#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>
#include <stdexcept>

#include "domain/types.hpp"
#include "schema/field.hpp"

namespace anime::domain {
using anime::schema::UPDATE_OPERATION;
using anime::schema::FieldInfo;
using anime::storage::recordField;

using GroupId = uint32_t;

struct Group {
	GroupId id = 0;
	std::string name;
	std::string description;
	std::vector<UserId> members;
};

struct GroupField {
	FieldInfo info;
	std::variant<GroupId Group::*, std::string Group::*, std::vector<UserId> Group::*> ptr;
};

inline const std::array<GroupField, 4> groupMetadata = { {
	{{}, },
	{{}, },
	{{}, },
	{{}, }
} };

inline const std::unordered_map<std::string, size_t> groupFieldNames = [] {
	std::unordered_map<std::string, size_t> result;
	for (size_t i = 0; i < groupMetadata.size(); ++i) result.emplace(groupMetadata[i].info.name, i);
	return result;
}();

struct GroupUpdate {
	GroupId id;
	UPDATE_OPERATION op;
	size_t fieldIndex;
	recordField value;
};

} // namespace anime::domain

