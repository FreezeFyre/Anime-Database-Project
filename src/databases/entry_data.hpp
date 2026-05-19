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
#include <array>
#include <typeinfo>

#include "../io/fileHandling.hpp"
#include "../filtering/filterProcessor.hpp"
#include "../net/protocol/data.hpp"

struct Context;
struct Message;
struct RouteResult;

struct entryData {
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

using intPTR = int entryData::*;
using strPTR = std::string entryData::*;
using vecPTR = std::vector<int> entryData::*;
using memPTR = std::variant< intPTR, vecPTR, strPTR>;

enum class fieldPresence {
	REQUIRED,
	RECOMMENDED,
	FIELD_OPTIONAL
};
struct fieldData {
	std::string name;
	memPTR ptr;
	schemaField::dataType type;
	schemaField::filterMode mode;
	fieldPresence presence;
};

// This Provides the Meta Data for each field in the entryData for use in other processes
inline const std::array<fieldData, 15> animeMeta = { {
	{"id",            &entryData::id,            schemaField::dataType::U32,     schemaField::filterMode::FM_NONE,         fieldPresence::REQUIRED},
	{"rank",          &entryData::rank,          schemaField::dataType::U32,     schemaField::filterMode::FM_RMV,          fieldPresence::REQUIRED},
	{"title",         &entryData::title,         schemaField::dataType::STR,     schemaField::filterMode::FM_NONE,         fieldPresence::REQUIRED},
	{"alt_title",     &entryData::alt_title,     schemaField::dataType::STR,     schemaField::filterMode::FM_NONE,         fieldPresence::FIELD_OPTIONAL},
	{"format",        &entryData::format,        schemaField::dataType::U8,      schemaField::filterMode::FM_MULTI_VALUE,  fieldPresence::FIELD_OPTIONAL},
	{"status",        &entryData::status,        schemaField::dataType::U8,      schemaField::filterMode::FM_MULTI_VALUE,  fieldPresence::FIELD_OPTIONAL},
	{"genres",        &entryData::genres,        schemaField::dataType::VECU8,   schemaField::filterMode::FM_MULTI_VALUE,  fieldPresence::FIELD_OPTIONAL},
	{"channels",      &entryData::channels,      schemaField::dataType::VECU8,   schemaField::filterMode::FM_MULTI_VALUE,  fieldPresence::FIELD_OPTIONAL},
	{"audios",        &entryData::audios,        schemaField::dataType::VECU8,   schemaField::filterMode::FM_MULTI_VALUE,  fieldPresence::FIELD_OPTIONAL},
	{"subtitles",     &entryData::subtitles,     schemaField::dataType::VECU8,   schemaField::filterMode::FM_MULTI_VALUE,  fieldPresence::FIELD_OPTIONAL},
	{"fan_service",   &entryData::fan_service,   schemaField::dataType::U8,      schemaField::filterMode::FM_RANGE,        fieldPresence::FIELD_OPTIONAL},
	{"episode_count", &entryData::episode_count, schemaField::dataType::U8,      schemaField::filterMode::FM_RANGE,        fieldPresence::FIELD_OPTIONAL},
	{"min_runtime",   &entryData::min_runtime,   schemaField::dataType::U16,     schemaField::filterMode::FM_RANGE,        fieldPresence::FIELD_OPTIONAL},
	{"characters",    &entryData::characters,    schemaField::dataType::VECU16,  schemaField::filterMode::FM_MULTI_VALUE,  fieldPresence::FIELD_OPTIONAL},
	{"ip_group",      &entryData::ip_group,      schemaField::dataType::U8,      schemaField::filterMode::FM_MULTI_VALUE,  fieldPresence::FIELD_OPTIONAL}
} };

std::unordered_map<std::string, size_t> buildFieldNames();
inline const std::unordered_map<std::string, size_t> nameMap = buildFieldNames();

/*
This provides us with the index of animeMeta that corresponds to the current index field in schemaData,
ie, schemaData[0] = fieldMap[0]

prevents errors caused by schema and animeMeta field order differences
*/


struct animeTables {
	std::vector<std::string> genres;
	std::vector<std::string> channels;
	std::vector<std::string> audios;
	std::vector<std::string> subtitles;
	std::vector<std::string> formats;
	std::vector<std::string> statuses;
	std::vector<std::string> medias;
};

class entryDB {
	public:

		// File Parsing Functions
		void loadEntries(std::string file);
		void loadLookupTables(std::string file);

		void saveEntries(std::string file);

		// File Handlers
		void record(recordData& record, schemaData& schema);
		void schema(schemaData& schema);
		void section(sectionHeader& header);
		void table(std::string& table, std::string& title);

		void addEntry(const entryData& entry) { entryData copy = entry; copy.id = database.size(); database.push_back(copy); }

		std::vector<size_t> getFilteredIDS(const indexFilters& filters);
		std::vector<int> getSortedIDS();


		// File Saving Functions
		fileHeader getHeader(fileType type);
		sectionHeader getSection(fileType type, size_t sectionIndex);
		schemaData getSchema(fileType type, size_t sectionIndex);
		recordData getRecord(fileType type, size_t sectionIndex, size_t recordIndex);
		recordData getTableRecord(fileType type, size_t sectionIndex, size_t rowIndex);

		entryData getEntry(size_t id) { return database[id]; }

		// Utility and Filtering Functions
		size_t getSize() { return database.size(); }
		recordField getField(size_t id, size_t fieldIndex);
		size_t getFieldCount() { return animeMeta.size(); }

	private:

		using data = std::vector<entryData>;

		data database;

		animeTables lookup;

		int lookupCount = 7;

		std::vector<size_t> fieldMap;
};

class entrySerializer {

	public:
		std::vector<uint8_t> payload(entryDB& db, const std::vector<size_t>& ids);

		std::vector<uint8_t> Entry(const entryData& entry);

		std::vector<uint8_t> Schema(const schemaData& schema);
		std::vector<uint8_t> Section(const sectionHeader& header);

		entryData readEntry(ByteReader& reader);

		schemaData readSchema(ByteReader& reader);

		sectionHeader readSection(ByteReader& reader);

};

class entryRouter {
	public:
		RouteResult route(Message& message, Context& context);
	private:
};
