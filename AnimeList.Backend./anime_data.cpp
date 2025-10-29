//
//  Anime_data.cpp
//  Anime List
//
//  Created by Luke Raykovitz on 10/24/25.
//

#include "anime_data.hpp"

std::unordered_map<size_t, animeDatabase> anime;

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
