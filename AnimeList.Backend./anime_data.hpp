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

#include "filters.hpp"

#pragma once

/* Anime_data_hpp */

struct animeDatabase {
    int rank;
    int media_type;
    std::string name;
    std::string alt_name;
    std::vector<int> service_id;
    std::vector<int> genre_ids;
    int status_id;
    std::vector<int> audio_lang_ids;
    std::vector<int> sub_lang_ids;
    int fan_service_rating;
    int season_count;
    int episode_count;
    int avg_episode_length;
    int hour_runtime;
    int min_runtime;
    std::vector<int> related_show_ids;
    std::vector<int> media_versions;
};

extern std::unordered_map<size_t, animeDatabase> anime;

using intMemberPTR = int animeDatabase::*;
using vecMemberPTR = std::vector<int> animeDatabase::*;

constexpr std::array<vecMemberPTR, SK_COUNT> setVecMembers = {
    nullptr,
    nullptr,
    &animeDatabase::service_id,
    &animeDatabase::genre_ids,
    nullptr,
    &animeDatabase::audio_lang_ids,
    &animeDatabase::sub_lang_ids,
    &animeDatabase::media_versions
};

constexpr std::array<intMemberPTR, SK_COUNT> setIntMembers = {
    nullptr,
    &animeDatabase::media_type,
    nullptr,
    nullptr,
    &animeDatabase::status_id,
    nullptr,
    nullptr,
    nullptr
};

constexpr std::array<intMemberPTR, RK_COUNT> rankMembers = {
    &animeDatabase::rank,
    &animeDatabase::fan_service_rating,
    &animeDatabase::season_count,
    &animeDatabase::episode_count,
    &animeDatabase::avg_episode_length,
    &animeDatabase::hour_runtime,
    &animeDatabase::min_runtime
};

/* Convert Following to a File Parser (metadata types saved in file)*/
extern const std::vector<std::string> service;

extern const std::vector<std::string> format;

extern const std::vector<std::string> genre;

extern const std::vector<std::string> status;

extern const std::vector<std::string> audio;

extern const std::vector<std::string> sub;

extern const std::vector<std::string> media;



void parseAnimeDATA(const std::string& file);

bool continuityCheck();
