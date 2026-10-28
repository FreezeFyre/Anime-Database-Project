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

using anime::schema::FIELD_PRESENCE;
using anime::schema::VALUE_REFERENCE_TYPE;
using anime::schema::UPDATE_OPERATION;
using anime::schema::Bound;
using anime::schema::FieldInfo;
using anime::storage::schemaField;
using anime::domain::GroupId;

struct Entry {
	EntryId id = 0;
	std::uint32_t rank = 0;
	std::string title = "";
	std::string alt_title = "";
	std::uint8_t format = 0;
	std::uint8_t status = 0;
	std::vector<std::uint8_t> genres = {};
	std::vector<std::uint8_t> channels = {};
	std::vector<std::uint8_t> audios = {};
	std::vector<std::uint8_t> subtitles = {};
	std::uint8_t fan_service = 0;
	std::uint16_t episode_count = 0;
	std::uint32_t min_runtime = 0;
	std::vector<std::uint32_t> characters = {};
	GroupId ip_group = 0;
};

using IndexStructure = std::unordered_map<std::string, std::unordered_map<LookupId, std::vector<EntryId>>>;

using memPTR = std::variant<
	std::uint8_t Entry::*,
	std::uint16_t Entry::*,
	std::uint32_t Entry::*,
	std::uint64_t Entry::*,
	std::string Entry::*,
	bool Entry::*,
	std::vector<std::uint8_t> Entry::*,
	std::vector<std::uint32_t> Entry::*
>;

struct EntryField {
	FieldInfo info;
	memPTR ptr;
};

// This Provides the Meta Data for each field in the Entry for use in other processes
inline const std::array<EntryField, 15> entryMetadata = { {
	{{"id",             schemaField::dataType::U32,     schemaField::filterMode::FM_NONE,         FIELD_PRESENCE::REQUIRED,       VALUE_REFERENCE_TYPE::NONE,         std::nullopt, ""          }, &Entry::id,           },
	{{"rank",           schemaField::dataType::U32,     schemaField::filterMode::FM_RMV,          FIELD_PRESENCE::REQUIRED,       VALUE_REFERENCE_TYPE::NONE,         std::nullopt, ""          }, &Entry::rank,         },
	{{"title",          schemaField::dataType::STR,     schemaField::filterMode::FM_NONE,         FIELD_PRESENCE::REQUIRED,       VALUE_REFERENCE_TYPE::NONE,         std::nullopt, ""          }, &Entry::title,        },
	{{"alt_title",      schemaField::dataType::STR,     schemaField::filterMode::FM_NONE,         FIELD_PRESENCE::FIELD_OPTIONAL, VALUE_REFERENCE_TYPE::NONE,         std::nullopt, ""          }, &Entry::alt_title,    },
	{{"format",         schemaField::dataType::U8,      schemaField::filterMode::FM_MULTI_VALUE,  FIELD_PRESENCE::FIELD_OPTIONAL, VALUE_REFERENCE_TYPE::LOOKUP_TABLE, std::nullopt, "format"    }, &Entry::format,       },
	{{"status",         schemaField::dataType::U8,      schemaField::filterMode::FM_MULTI_VALUE,  FIELD_PRESENCE::FIELD_OPTIONAL, VALUE_REFERENCE_TYPE::LOOKUP_TABLE, std::nullopt, "status"    }, &Entry::status,       },
	{{"genres",         schemaField::dataType::VECU8,   schemaField::filterMode::FM_MULTI_VALUE,  FIELD_PRESENCE::FIELD_OPTIONAL, VALUE_REFERENCE_TYPE::LOOKUP_TABLE, std::nullopt, "genres"    }, &Entry::genres,       },
	{{"channels",       schemaField::dataType::VECU8,   schemaField::filterMode::FM_MULTI_VALUE,  FIELD_PRESENCE::FIELD_OPTIONAL, VALUE_REFERENCE_TYPE::LOOKUP_TABLE, std::nullopt, "channels"  }, &Entry::channels,     },
	{{"audios",         schemaField::dataType::VECU8,   schemaField::filterMode::FM_MULTI_VALUE,  FIELD_PRESENCE::FIELD_OPTIONAL, VALUE_REFERENCE_TYPE::LOOKUP_TABLE, std::nullopt, "audios"    }, &Entry::audios,       },
	{{"subtitles",      schemaField::dataType::VECU8,   schemaField::filterMode::FM_MULTI_VALUE,  FIELD_PRESENCE::FIELD_OPTIONAL, VALUE_REFERENCE_TYPE::LOOKUP_TABLE, std::nullopt, "subtitles" }, &Entry::subtitles,    },
	{{"fan_service",    schemaField::dataType::U8,      schemaField::filterMode::FM_RANGE,        FIELD_PRESENCE::FIELD_OPTIONAL, VALUE_REFERENCE_TYPE::BOUNDS,       Bound{0, 10}, ""          }, &Entry::fan_service,  },
	{{"episode_count",  schemaField::dataType::U16,      schemaField::filterMode::FM_RANGE,        FIELD_PRESENCE::FIELD_OPTIONAL, VALUE_REFERENCE_TYPE::NONE,         std::nullopt, ""          }, &Entry::episode_count,},
	{{"min_runtime",    schemaField::dataType::U32,     schemaField::filterMode::FM_RANGE,        FIELD_PRESENCE::FIELD_OPTIONAL, VALUE_REFERENCE_TYPE::NONE,         std::nullopt, ""          }, &Entry::min_runtime,  },
	{{"characters",     schemaField::dataType::VECU32,  schemaField::filterMode::FM_MULTI_VALUE,  FIELD_PRESENCE::FIELD_OPTIONAL, VALUE_REFERENCE_TYPE::NONE,         std::nullopt, ""          }, &Entry::characters,   },
	{{"ip_group",       schemaField::dataType::U32,      schemaField::filterMode::FM_MULTI_VALUE,  FIELD_PRESENCE::REQUIRED, VALUE_REFERENCE_TYPE::FOREIGN_RECORD,         std::nullopt, "Group"          }, &Entry::ip_group,     }
} };


inline const std::unordered_map<std::string, size_t> entryFieldNames = [] {
	std::unordered_map<std::string, size_t> result;
	for (size_t i = 0; i < entryMetadata.size(); ++i) result.emplace(entryMetadata[i].info.name, i);
	return result;
}();

/*
This provides us with the index of EntryMeta that corresponds to the current index field in schemaData,
ie, schemaData[0] = fieldMap[0]

prevents errors caused by schema and EntryMeta field order differences
*/

struct EntryUpdate {
	size_t id;
	UPDATE_OPERATION op;
	size_t fieldIndex;
	anime::storage::recordField value;
};

} // namespace anime::domain

