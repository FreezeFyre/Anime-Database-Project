//
//  Anime_data.hpp
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

#include "Errors.h"
#include "fileParsing.h"

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

using animeMemPTR = std::variant<intPTR, vecPTR, strPTR>;

std::unordered_map<std::string, animeMemPTR> animeFieldMap = {
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

using filteredIDS = std::vector<int>;

class animeDB {
	public:
		void loadAnimes(std::string file);

		void addAnime(schemaData& schema, recordData& record);

		databaseErrors validateMap();

		filteredIDS processFilters(const Filters& f);

	private:

		using animeMap = std::unordered_map<int, animeData>;

		using setsVariantPTR = std::variant<intPTR, vecPTR>;

		animeMap animeDatabase;
};




constexpr std::array<setsVariantPTR, SK_COUNT> setsPTR = {
	&animeData::media_type,    // intPTR
	&animeData::service_id,    // vecPTR
	&animeData::genre_ids,     // vecPTR
	&animeData::status_id,     // intPTR
	&animeData::audio_lang_ids,// vecPTR
	&animeData::sub_lang_ids,  // vecPTR
	&animeData::media_versions // vecPTR
};

/* Range Pointers Are Completed*/
constexpr std::array<intPTR, RK_COUNT> rangeIntPTR   = {
	&animeData::rank,
	&animeData::fan_service_rating,
	&animeData::season_count,
	&animeData::episode_count,
	&animeData::avg_episode_length,
	&animeData::hour_runtime,
	&animeData::min_runtime
};

using storeVariantPTR = std::variant<intPTR, vecPTR, strPTR>;

constexpr std::array<storeVariantPTR, 17> animePtr = {
	&animeData::rank,
	&animeData::media_type,
	&animeData::name,
	&animeData::alt_name,
	&animeData::service_id,
	&animeData::genre_ids,
	&animeData::status_id,
	&animeData::audio_lang_ids,
	&animeData::sub_lang_ids,
	&animeData::fan_service_rating,
	&animeData::season_count,
	&animeData::episode_count,
	&animeData::avg_episode_length,
	&animeData::hour_runtime,
	&animeData::min_runtime,
	&animeData::related_show_ids,
	&animeData::media_versions
};


/* Convert Following to a File Parser (metadata types saved in file)*/
extern const std::vector<std::string> service;

extern const std::vector<std::string> format;

extern const std::vector<std::string> genre;

extern const std::vector<std::string> status;

extern const std::vector<std::string> audio;

extern const std::vector<std::string> sub;

extern const std::vector<std::string> media;

using stringVec = const std::vector<std::string>;

static const std::array<stringVec*, SK_COUNT> filterSets = {
	&service,
	&format,
	&genre,
	&status,
	&audio,
	&sub,
	&media
};

void parseAnimes(const std::string& file);

void storeAnimeData(int phase);

bool continuityCheck();
