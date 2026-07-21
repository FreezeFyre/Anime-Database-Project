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

#include "../io/fileHandling.hpp"
#include "../core.hpp"
#include "users.hpp"
#include "../filtering/filterProcessor.hpp"
#include "../net/protocol/data.hpp"
#include "../net/protocol/messenger.hpp"
#include "../net/transport/session.hpp"

struct Context;
struct Message;
struct RouteResult;

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
inline const std::array<EntryField, 15> EntryMeta = { {
	{{"id",             schemaField::dataType::U32,     schemaField::filterMode::FM_NONE,         fieldPresence::REQUIRED,       ValueRef::NONE,         std::nullopt, ""          }, &Entry::id,           },
	{{"rank",           schemaField::dataType::U32,     schemaField::filterMode::FM_RMV,          fieldPresence::REQUIRED,       ValueRef::NONE,         std::nullopt, ""          }, &Entry::rank,         },
	{{"title",          schemaField::dataType::STR,     schemaField::filterMode::FM_NONE,         fieldPresence::REQUIRED,       ValueRef::NONE,         std::nullopt, ""          }, &Entry::title,        },
	{{"alt_title",      schemaField::dataType::STR,     schemaField::filterMode::FM_NONE,         fieldPresence::FIELD_OPTIONAL, ValueRef::NONE,         std::nullopt, ""          }, &Entry::alt_title,    },
	{{"format",         schemaField::dataType::U8,      schemaField::filterMode::FM_MULTI_VALUE,  fieldPresence::FIELD_OPTIONAL, ValueRef::LOOKUP_TABLE, std::nullopt, "format"    }, &Entry::format,       },
	{{"status",         schemaField::dataType::U8,      schemaField::filterMode::FM_MULTI_VALUE,  fieldPresence::FIELD_OPTIONAL, ValueRef::LOOKUP_TABLE, std::nullopt, "status"    }, &Entry::status,       },
	{{"genres",         schemaField::dataType::VECU8,   schemaField::filterMode::FM_MULTI_VALUE,  fieldPresence::FIELD_OPTIONAL, ValueRef::LOOKUP_TABLE, std::nullopt, "genres"    }, &Entry::genres,       },
	{{"channels",       schemaField::dataType::VECU8,   schemaField::filterMode::FM_MULTI_VALUE,  fieldPresence::FIELD_OPTIONAL, ValueRef::LOOKUP_TABLE, std::nullopt, "channels"  }, &Entry::channels,     },
	{{"audios",         schemaField::dataType::VECU8,   schemaField::filterMode::FM_MULTI_VALUE,  fieldPresence::FIELD_OPTIONAL, ValueRef::LOOKUP_TABLE, std::nullopt, "audios"    }, &Entry::audios,       },
	{{"subtitles",      schemaField::dataType::VECU8,   schemaField::filterMode::FM_MULTI_VALUE,  fieldPresence::FIELD_OPTIONAL, ValueRef::LOOKUP_TABLE, std::nullopt, "subtitles" }, &Entry::subtitles,    },
	{{"fan_service",    schemaField::dataType::U8,      schemaField::filterMode::FM_RANGE,        fieldPresence::FIELD_OPTIONAL, ValueRef::BOUNDS,       Bound{0, 10}, ""          }, &Entry::fan_service,  },
	{{"episode_count",  schemaField::dataType::U8,      schemaField::filterMode::FM_RANGE,        fieldPresence::FIELD_OPTIONAL, ValueRef::NONE,         std::nullopt, ""          }, &Entry::episode_count,},
	{{"min_runtime",    schemaField::dataType::U16,     schemaField::filterMode::FM_RANGE,        fieldPresence::FIELD_OPTIONAL, ValueRef::NONE,         std::nullopt, ""          }, &Entry::min_runtime,  },
	{{"characters",     schemaField::dataType::VECU16,  schemaField::filterMode::FM_MULTI_VALUE,  fieldPresence::FIELD_OPTIONAL, ValueRef::NONE,         std::nullopt, ""          }, &Entry::characters,   },
	{{"ip_group",       schemaField::dataType::U8,      schemaField::filterMode::FM_MULTI_VALUE,  fieldPresence::FIELD_OPTIONAL, ValueRef::NONE,         std::nullopt, ""          }, &Entry::ip_group,     }
} };


std::unordered_map<std::string, size_t> buildFieldNames(); 
inline const std::unordered_map<std::string, size_t> nameMap = buildFieldNames();

/*
This provides us with the index of EntryMeta that corresponds to the current index field in schemaData,
ie, schemaData[0] = fieldMap[0]

prevents errors caused by schema and EntryMeta field order differences
*/

struct EntryUpdate {
	size_t id;
	UpdateOp op;
	size_t fieldIndex;
	recordField value;
};


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
		schemaData getSchema(fileType type, size_t sectionIndex);
		recordData getRecord(fileType type, size_t sectionIndex, size_t recordIndex);
		recordData getTableRecord(fileType type, size_t sectionIndex, size_t rowIndex);
		EntryId getId(size_t index) const {
			return static_cast<EntryId>(database[index].id);
		}
	    size_t getFieldIndex(const std::string& fieldName) const {
		    auto it = nameMap.find(fieldName);
		    if (it != nameMap.end()) {
			    return it->second;
		    }
		    throw std::runtime_error("Field name not found: " + fieldName);
	    }

		// Entry operations
		void addEntry(const Entry& entry, const UserRole& role);
		void updateEntry(const std::vector<EntryUpdate>& updates, const UserRole& role);
		std::vector<Entry> getEntry(std::vector<EntryId> ids);
		void deleteEntry(const EntryId id, const UserRole& role);
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
			return EntryMeta[fieldIndex].info;
		}
		size_t getSize() const { return database.size(); }
		recordField getField(size_t id, size_t fieldIndex) const;
		recordField getFromEntry(const Entry& entry, const size_t fieldIndex) const;
		size_t getFieldCount() const { return EntryMeta.size(); }
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

class entrySerializer {

	public:
		std::vector<uint8_t> payload(entryDB& db, const std::vector<EntryId>& ids);

		std::vector<uint8_t> serializeEntry(const Entry& entry);

		std::vector<uint8_t> Schema(const schemaData& schema);
		std::vector<uint8_t> Section(const sectionHeader& header);

		Entry readEntry(ByteReader& reader, std::vector<size_t>& fieldIndices);

		schemaData readSchema(ByteReader& reader);

		sectionHeader readSection(ByteReader& reader);

		EntryUpdate translateUpdate(const Update& update);


};

class entryRouter {
	public:
		RouteResult route(Message& message, Context& context, Session& session);
	private:
};
