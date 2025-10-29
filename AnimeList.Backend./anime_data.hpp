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
    float avg_episode_length;
    int hour_runtime;
    int min_runtime;
    std::vector<int> related_show_ids;
    std::vector<int> media_versions;
};



/* Convert Following to a File Parser (metadata types saved in file)*/
const std::vector<std::string> service = {
    "Crunchyroll",
    "Netflix",
    "Hulu",
    "Disney Plus",
    "Amazon Prime Video",
    "HiDive",
    "Youtube"
};

const std::vector<std::string> format = {
    "Series",
    "Movie"
};

const std::vector<std::string> genre = {
    "Action",
    "Romance",
    "Slice of Life",
    "Comedy",
    "Drama",
    "Highschool",
    "College",
    "SciFi",
    "Modern Day",
    "Fantasy",
    "Ecchi",
    "Moving",
    "Magic",
    "Coming of Age",
    "Adventure",
    "Mystery",
    "Thriller",
    "Horror",
    "Supernatural",
    "Historical",
    "Mech",
    "Psychological",
    "Military",
    "Sports",
    "Music",
    "Martial Arts",
    "Shounen",
    "Shoujo",
    "Seinen",
    "Josei",
    "Isekai"
};

const std::vector<std::string> status = {
    "Ongoing",
    "Completed",
    "Limbo",
    "Cancelled",
    "Un-Released"
};

const std::vector<std::string> audio = {
    "Japanese",
    "English",
    "Korean",
    "Chinese",
    "Spanish",
    "French"
};

const std::vector<std::string> sub = {
    "Japanese",
    "English",
    "Korean",
    "Chinese",
    "Spanish",
    "French"
};

const std::vector<std::string> media = {
    "Manga",
    "Light Novel",
    "Movie",
    "Series",
    "Game"
};
