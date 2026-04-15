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
#include "../net/data.hpp"

#pragma once


struct animeData {
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

using intPTR = int animeData::*;

using strPTR = std::string animeData::*;

using vecPTR = std::vector<int> animeData::*;

using memPTR = std::variant< intPTR, vecPTR, strPTR>;

enum class fieldPresence {
	REQUIRED,
	RECOMMENDED,
	OPTIONAL
};

struct fieldData {
	std::string name;
	memPTR ptr;
	schemaField::dataType type;
	schemaField::filterMode mode;
	fieldPresence presence;
};

// This Provides the Meta Data for each field in the animeData for use in other processes
inline const std::array<fieldData, 15> animeMeta = { {
	{"id",            &animeData::id,            schemaField::dataType::U32,     schemaField::filterMode::FM_NONE,         fieldPresence::REQUIRED},
	{"rank",          &animeData::rank,          schemaField::dataType::U32,     schemaField::filterMode::FM_RMV,          fieldPresence::REQUIRED},
	{"title",         &animeData::title,         schemaField::dataType::STR,     schemaField::filterMode::FM_NONE,         fieldPresence::REQUIRED},
	{"alt_title",     &animeData::alt_title,     schemaField::dataType::STR,     schemaField::filterMode::FM_NONE,         fieldPresence::OPTIONAL},
	{"format",        &animeData::format,        schemaField::dataType::U8,      schemaField::filterMode::FM_MULTI_VALUE,  fieldPresence::OPTIONAL},
	{"status",        &animeData::status,        schemaField::dataType::U8,      schemaField::filterMode::FM_MULTI_VALUE,  fieldPresence::OPTIONAL},
	{"genres",        &animeData::genres,        schemaField::dataType::VECU8,   schemaField::filterMode::FM_MULTI_VALUE,  fieldPresence::OPTIONAL},
	{"channels",      &animeData::channels,      schemaField::dataType::VECU8,   schemaField::filterMode::FM_MULTI_VALUE,  fieldPresence::OPTIONAL},
	{"audios",        &animeData::audios,        schemaField::dataType::VECU8,   schemaField::filterMode::FM_MULTI_VALUE,  fieldPresence::OPTIONAL},
	{"subtitles",     &animeData::subtitles,     schemaField::dataType::VECU8,   schemaField::filterMode::FM_MULTI_VALUE,  fieldPresence::OPTIONAL},
	{"fan_service",   &animeData::fan_service,   schemaField::dataType::U8,      schemaField::filterMode::FM_RANGE,        fieldPresence::OPTIONAL},
	{"episode_count", &animeData::episode_count, schemaField::dataType::U8,      schemaField::filterMode::FM_RANGE,        fieldPresence::OPTIONAL},
	{"min_runtime",   &animeData::min_runtime,   schemaField::dataType::U16,     schemaField::filterMode::FM_RANGE,        fieldPresence::OPTIONAL},
	{"characters",    &animeData::characters,    schemaField::dataType::VECU16,  schemaField::filterMode::FM_MULTI_VALUE,  fieldPresence::OPTIONAL},
	{"ip_group",      &animeData::ip_group,      schemaField::dataType::U8,      schemaField::filterMode::FM_MULTI_VALUE,  fieldPresence::OPTIONAL}
} };


std::unordered_map<std::string, size_t> buildFieldNames();

inline const std::unordered_map<std::string, size_t> nameMap = buildFieldNames();



/*
This provides us with the index of animeMeta that coresponds to the current index field in schemaData,
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

class animeDB {
	public:

		// File Parsing Functions
		void loadAnimes(std::string file);

		void loadLookupTables(std::string file);

		// File Handlers
		void record(recordData& record, schemaData& schema);

		void schema(schemaData& schema);

		void section(sectionHeader& header);

		void table(std::string& table, std::string& title);



		std::vector<size_t> getFilteredIDS(const indexFilters& filters);

		std::vector<int> getSortedIDS();

		// File Saving Functions
		fileHeader getHeader();

		sectionHeader getSection();

		schemaData getSchema();

		recordData getRecord();


		// Utility and Filtering Functions
		size_t getSize() {
			return database.size();
		}

		recordField getField(size_t id, size_t fieldIndex);

		size_t getFieldCount() {
			return animeMeta.size();
		}


	private:

		using data = std::vector<animeData>;

		data database;

		animeTables lookup;

		std::vector<size_t> fieldMap;
};