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

using memPTR = std::variant<intPTR, vecPTR, strPTR>;

std::unordered_map<std::string, memPTR> animeFieldMap = {
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

std::vector<memPTR> orderedMemPTRS;

class animeDB {
	public:
		void loadAnimes(std::string file);

		void record(recordData& record);

		void schema(schemaData& schema);

		void header(fileHeader& header);

		std::vector<int> getFilteredIDS(const Filters& f);

		std::vector<int> getSortedIDS();

		void getData();
	private:

		using data = std::vector<animeData>;

		data database;
};


/* Convert Following to a File Parser (metadata types saved in file)*/
extern const std::vector<std::string> service;

extern const std::vector<std::string> format;

extern const std::vector<std::string> genre;

extern const std::vector<std::string> status;

extern const std::vector<std::string> audio;

extern const std::vector<std::string> sub;

extern const std::vector<std::string> media;