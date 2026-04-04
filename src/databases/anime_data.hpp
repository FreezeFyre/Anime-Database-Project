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
	int rank;
	std::string title;
	std::string alt_title;
	int format;
	int status;
	std::vector<int> genres;
	std::vector<int> channels;
	std::vector<int> audios;
	std::vector<int> subtitles;
	int fan_service;
	int episode_count;
	int min_runtime;
	std::vector<int> characters;
	int ip_group;
};

using intPTR = int animeData::*;

using vecPTR = std::vector<int> animeData::*;

using strPTR = std::string animeData::*;

using memPTR = std::variant<intPTR, vecPTR, strPTR>;

struct fieldData {
	std::string name;
	memPTR ptr;
	schemaField::dataType type;
	schemaField::filterMode mode;
};

// This Provides the Meta Data for each field in the animeData for use in other processes
inline const std::array<fieldData, 14> animeMeta = { {
	{"rank",          &animeData::rank,          schemaField::dataType::U32,     schemaField::filterMode::FM_RMV},
	{"title",         &animeData::title,         schemaField::dataType::STR,     schemaField::filterMode::FM_NONE},
	{"alt_title",     &animeData::alt_title,     schemaField::dataType::STR,     schemaField::filterMode::FM_NONE},
	{"format",        &animeData::format,        schemaField::dataType::U8,      schemaField::filterMode::FM_MULTI_VALUE},
	{"status",        &animeData::status,        schemaField::dataType::U8,      schemaField::filterMode::FM_MULTI_VALUE},
	{"genres",        &animeData::genres,        schemaField::dataType::VECU8,   schemaField::filterMode::FM_MULTI_VALUE},
	{"channels",      &animeData::channels,      schemaField::dataType::VECU8,   schemaField::filterMode::FM_MULTI_VALUE},
	{"audios",        &animeData::audios,        schemaField::dataType::VECU8,   schemaField::filterMode::FM_MULTI_VALUE},
	{"subtitles",     &animeData::subtitles,     schemaField::dataType::VECU8,   schemaField::filterMode::FM_MULTI_VALUE},
	{"fan_service",   &animeData::fan_service,   schemaField::dataType::U8,      schemaField::filterMode::FM_RANGE},
	{"episode_count", &animeData::episode_count, schemaField::dataType::U8,      schemaField::filterMode::FM_RANGE},
	{"min_runtime",   &animeData::min_runtime,   schemaField::dataType::U16,     schemaField::filterMode::FM_RANGE},
	{"characters",    &animeData::characters,    schemaField::dataType::VECU16,  schemaField::filterMode::FM_MULTI_VALUE},
	{"ip_group",      &animeData::ip_group,      schemaField::dataType::U8,      schemaField::filterMode::FM_MULTI_VALUE}
} };

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
};

class animeSerializer {
	public:
		static std::vector<uint8_t> pack(const animeData& anime);

		static animeData unpack(const std::vector<uint8_t>& bytes);
};
