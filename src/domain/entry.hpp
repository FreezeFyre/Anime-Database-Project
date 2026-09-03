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

using namespace anime::schema;
using anime::storage::schemaField;

struct Entry {
	int id = 0; // Safety default only; missing input must still be validated.
	int rank = 0;
	std::string title = "";
	std::string alt_title = "";
	int format = 0; // Safety default only; 0 is also a valid lookup-table index.
	int status = 0; // Safety default only; 0 is also a valid lookup-table index.
	std::vector<int> genres = {};
	std::vector<int> channels = {};
	std::vector<int> audios = {};
	std::vector<int> subtitles = {};
	int fan_service = 0;
	int episode_count = 0;
	int min_runtime = 0;
	std::vector<int> characters = {};
	int ip_group = 0;  // Safety default only; 0 may be a real group id later.
};

using EntryId = uint32_t;
using LookupId = uint32_t;

using IndexStructure = std::unordered_map<std::string, std::unordered_map<LookupId, std::vector<EntryId>>>;

using intPTR = int Entry::*;
using strPTR = std::string Entry::*;
using vecPTR = std::vector<int> Entry::*;
using memPTR = std::variant< intPTR, vecPTR, strPTR>;

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
	{{"episode_count",  schemaField::dataType::U8,      schemaField::filterMode::FM_RANGE,        FIELD_PRESENCE::FIELD_OPTIONAL, VALUE_REFERENCE_TYPE::NONE,         std::nullopt, ""          }, &Entry::episode_count,},
	{{"min_runtime",    schemaField::dataType::U16,     schemaField::filterMode::FM_RANGE,        FIELD_PRESENCE::FIELD_OPTIONAL, VALUE_REFERENCE_TYPE::NONE,         std::nullopt, ""          }, &Entry::min_runtime,  },
	{{"characters",     schemaField::dataType::VECU16,  schemaField::filterMode::FM_MULTI_VALUE,  FIELD_PRESENCE::FIELD_OPTIONAL, VALUE_REFERENCE_TYPE::NONE,         std::nullopt, ""          }, &Entry::characters,   },
	{{"ip_group",       schemaField::dataType::U8,      schemaField::filterMode::FM_MULTI_VALUE,  FIELD_PRESENCE::FIELD_OPTIONAL, VALUE_REFERENCE_TYPE::NONE,         std::nullopt, ""          }, &Entry::ip_group,     }
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

