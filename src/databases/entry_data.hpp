#pragma once
//
//  AnimeData.hpp
//  Anime List
//
//  Created by Luke Raykovitz on 10/24/25.
//
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
#include <unordered_set>
#include <array>
#include <typeinfo>
#include <optional>
#include <shared_mutex>
#include <mutex>

#include "io/fileHandling.hpp"
#include "domain/types.hpp"
#include "schema/field.hpp"
#include "protocol/message_types.hpp"
#include "filtering/filterProcessor.hpp"
#include "net/protocol/data.hpp"
#include "net/transport/session.hpp"

namespace anime::app { struct Context; }

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

namespace anime::storage {

using namespace anime::domain;
using namespace anime::schema;
using namespace anime::filtering;

class entryDB {
	public:
		using IdType = EntryId;

		// File load/save entry points
		void loadEntries(std::string file);
		void loadLookupTables(std::string file);
		void saveEntries(std::string file);


		// File read handlers
		void section(const sectionHeader& header);
		void schema(const sectionHeader& section, const schemaData& schema);
		void record(const sectionHeader& section, const recordData& record, const schemaData& schema);
		void table(const sectionHeader& section, const std::string& table);

		// File write sources
		fileHeader getHeader(fileType type);
		sectionHeader getSection(fileType type, size_t sectionIndex);
		sectionHeader getSection();
		schemaData getSchema(fileType type, size_t sectionIndex);
		schemaData getSchema();
		recordData getRecord(fileType type, size_t sectionIndex, size_t recordIndex);
		recordData getTableRecord(fileType type, size_t sectionIndex, size_t rowIndex);
		EntryId getId(size_t index) const {
			return static_cast<EntryId>(database[index].id);
		}
	    size_t getFieldIndex(const std::string& fieldName) const {
		    auto it = entryFieldNames.find(fieldName);
		    if (it != entryFieldNames.end()) {
			    return it->second;
		    }
		    throw std::runtime_error("Field name not found: " + fieldName);
	    }

		// Entry operations
		void addEntry(const Entry& entry, const USER_ACCOUNT_ROLE& role);
		void updateEntry(const std::vector<EntryUpdate>& updates, const USER_ACCOUNT_ROLE& role);
		std::vector<Entry> getEntries(std::vector<EntryId> ids);
		void deleteEntry(const EntryId id, const USER_ACCOUNT_ROLE& role);
		std::vector<EntryId> searchEntryTitles(const std::string& term);

		// Operation Utilities
		void setField(Entry& entry, size_t fieldIndex, const recordField& value);
		void addFieldValue(Entry& entry, size_t fieldIndex, const recordField& value);
		void removeFieldValue(Entry& entry, size_t fieldIndex, const recordField& value);
		void clearField(Entry& entry, size_t fieldIndex);
		std::string normalizeString(const std::string& string);
		std::vector<size_t> idToIndex(std::vector<EntryId> ids);

		// Query operations
		std::vector<EntryId> getFilteredIDS(const FilterRequest& filters) const;
		std::vector<EntryId> getSortedIDS();

		// Index construction
		void constructIdMap(std::unordered_map<EntryId, size_t>& idMap) const;
		void constructTableIndexes(IndexStructure& tableIndexes) const;
		EntryId addId();
		bool rankInsert(int rank);

		// Validation
		std::vector<size_t> validateSchema(const schemaData& schema);
		void validateTable(const std::string& title);
		void validateField(const recordField& value,const size_t fieldIndex);
		void validateScalar(const int value, const FieldInfo& meta);

		// Utility accessors
		const FieldInfo& getFieldInfo(size_t fieldIndex) const {
			return entryMetadata[fieldIndex].info;
		}
		size_t getSize() const { return database.size(); }
		recordField getField(size_t id, size_t fieldIndex) const;
		recordField getFromEntry(const Entry& entry, const size_t fieldIndex) const;
		size_t getFieldCount() const { return entryMetadata.size(); }
		size_t getTableSize(const std::string& title);

	private:
		std::unordered_map<EntryId, size_t> idMap; //Maps ids to their index in the vector allowing for database transforms
		EntryId nextId = 50;

		std::vector<Entry> database;
		std::vector<size_t> fieldMap;

		std::unordered_map<std::string, std::vector<std::string>> lookupTables;
		

		// Acceleration Structures
		IndexStructure TableIndexes;

};

} // namespace anime::storage

namespace anime::protocol {

using namespace anime::domain;
using anime::storage::schemaData;
using anime::storage::sectionHeader;
using anime::filtering::FilterRequest;

struct Update {
	std::uint32_t id;
	anime::schema::UPDATE_OPERATION operation;
	std::uint8_t fieldIndex;
	anime::storage::dataType type;
	std::variant<std::monostate, std::uint8_t, std::uint16_t, std::uint32_t,
		std::string, std::vector<int>> value;
};

struct EntryGetRequest {
	std::vector<EntryId> ids;
};
struct EntryUpdateRequest {
	std::vector<EntryUpdate> updates;
};
struct EntryCreateRequest { // No schema as its used only for translation
	std::vector<Entry> entries;
};
struct EntryDeleteRequest {
	std::vector<EntryId> ids;
};
struct EntrySearchRequest { // This will change once searches arent title only
	std::string searchTerm;
};
struct EntryFilterRequest {
	FilterRequest filters;
};
using EntryRequest = std::variant<
	EntryGetRequest,
	EntryUpdateRequest,
	EntryCreateRequest,
	EntryDeleteRequest,
	EntrySearchRequest,
	EntryFilterRequest
>;

struct EntryGetResponse {
	sectionHeader section;
	schemaData schema;
	std::vector<Entry> entries;
};
struct EntryIdsResponse {
    std::vector<EntryId> ids;
};
struct EntrySuccessResponse {
    std::string message = "SUCCESS";
};

struct EntryErrorResponse {
    std::string message;
};
using EntryResponsePayload = std::variant<
    EntryGetResponse,
    EntryIdsResponse,
    EntrySuccessResponse,
    EntryErrorResponse
>;
struct EntryResponse {
	EntryResponsePayload payload;
    bool closeConnection = false;
};

class EntryWebSocketCodec {
	public:
		EntryRequest decode(ACTION_TYPE action, uint32_t id, const ByteVector& payload, anime::app::Context& context, anime::net::Session& session);
		ByteVector encode(const EntryResponse& response);

	private:
		Entry readEntry(ByteReader& reader, std::vector<size_t>& fieldIndices);
		schemaData readSchema(ByteReader& reader);
		sectionHeader readSection(ByteReader& reader);
		EntryUpdate translateUpdate(const Update& update);

		void encodeSection(ByteWriter& encoder, const sectionHeader& section);
		void encodeSchema(ByteWriter& encoder, const schemaData& schema);
		void encodeEntry(ByteWriter& encoder, const Entry& entry);

		ByteVector encodeResponse(const EntryGetResponse& response);
		ByteVector encodeResponse(const EntryIdsResponse& response);
		ByteVector encodeResponse(const EntrySuccessResponse& response);
		ByteVector encodeResponse(const EntryErrorResponse& response);
};

} // namespace anime::protocol

namespace anime::app {

using namespace anime::protocol;
class EntryRouter {
	public:
		EntryResponse route(const EntryRequest& request, Context& context, anime::net::Session& session);

	private:
		EntryResponse handle(const EntryGetRequest& request, Context& context, anime::net::Session& session);
		EntryResponse handle(const EntryUpdateRequest& request, Context& context, anime::net::Session& session);
		EntryResponse handle(const EntryCreateRequest& request, Context& context, anime::net::Session& session);
		EntryResponse handle(const EntryDeleteRequest& request, Context& context, anime::net::Session& session);
		EntryResponse handle(const EntrySearchRequest& request, Context& context, anime::net::Session& session);
		EntryResponse handle(const EntryFilterRequest& request, Context& context, anime::net::Session& session);
};

} // namespace anime::app
