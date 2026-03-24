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

inline const std::unordered_map<std::string, memPTR> animeFieldMap = {
	{"rank",&animeData::rank},
	{"title",&animeData::title},
	{"alt_title",&animeData::alt_title},
	{"format",&animeData::format},
	{"status",&animeData::status},
	{"genres",&animeData::genres},
	{"channels",&animeData::channels},
	{"audios",&animeData::audios},
	{"subtitles",&animeData::subtitles},
	{"fan_service",&animeData::fan_service},
	{"episode_count",&animeData::episode_count},
	{"min_runtime",&animeData::min_runtime},
	{"characters",&animeData::characters},
	{"ip_group",&animeData::ip_group}
};

using fMode = schemaField::filterMode;
using dType = schemaField::dataType;

struct fieldData {
	std::string name;
	memPTR ptr;
	dType type;
	fMode mode;
};

inline const std::array<fieldData, 14> animeMeta = {
	{"rank",          &animeData::rank,          dType::U32,     fMode::FM_RMV},
	{"title",         &animeData::title,         dType::STR,     fMode::FM_NONE},
	{"alt_title",     &animeData::alt_title,     dType::STR,     fMode::FM_NONE},
	{"format",        &animeData::format,        dType::U8,      fMode::FM_MULTI_VALUE},
	{"status",        &animeData::status,        dType::U8,      fMode::FM_MULTI_VALUE},
	{"genres",        &animeData::genres,        dType::VECU8,   fMode::FM_MULTI_VALUE},
	{"channels",      &animeData::channels,      dType::VECU8,   fMode::FM_MULTI_VALUE},
	{"audios",        &animeData::audios,        dType::VECU8,   fMode::FM_MULTI_VALUE},
	{"subtitles",     &animeData::subtitles,     dType::VECU8,   fMode::FM_MULTI_VALUE},
	{"fan_service",   &animeData::fan_service,   dType::U8,      fMode::FM_RANGE},
	{"episode_count", &animeData::episode_count, dType::U8,      fMode::FM_RANGE},
	{"min_runtime",   &animeData::min_runtime,   dType::U16,     fMode::FM_RANGE},
	{"characters",    &animeData::characters,    dType::VECU16,  fMode::FM_MULTI_VALUE},
	{"ip_group",      &animeData::ip_group,      dType::U8,      fMode::FM_MULTI_VALUE}
};


class animeDB {
	public:
		void loadAnimes(std::string file);

		void record(recordData& record, schemaData& schema);

		void header(fileHeader& header);

		std::vector<size_t> getFilteredIDS(const indexFilters& filters);

		std::vector<int> getSortedIDS();

		fileHeader getHeader();

		schemaData getSchema();

		recordData getRecord();

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
};




class SerializeAnime {
	public:
		static std::vector<uint8_t> pack(const animeData& anime);

		static animeData unpack(const std::vector<uint8_t>& bytes);
};

/* Convert Following to a File Parser (metadata types saved in file)*/
extern const std::vector<std::string> service;

extern const std::vector<std::string> format;

extern const std::vector<std::string> genre;

extern const std::vector<std::string> status;

extern const std::vector<std::string> audio;

extern const std::vector<std::string> sub;

extern const std::vector<std::string> media;