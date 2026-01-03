//
//  Anime_data.cpp
//  Anime List
//
//  Created by Luke Raykovitz on 10/24/25.
//

#include "anime_data.hpp"

#include <vector>
#include <unordered_map>

std::unordered_map<size_t, animeData> anime;

databaseErrors animeDatabase::validateMap() {

    databaseErrors errors;

    for (int i = 0; i < anime.size(); i++) {
        if (!anime.count(i)) {
            errors.push_back({ mapError::errorType::NO_ID, i, "Missing dense ID" });
        }

        if (i >= anime.size()) {
            errors.push_back({ mapError::errorType::OUT_OF_RANGE, i, "Dense ID out of range" });
        }
    }

    return errors;
}


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


// Animes should have 18 phases (0-17)
void parseAnimes(const std::string& file) {
    int nest_level = 0;
    int phase = 0;
    int slot_index = 0;
    bool slot_collection = 0;
	bool in_string = 0;
    std::string buffer;
    std::string title_buffer;
    std::vector<int> slot_values;
    animeData animeBuffer;

    std::ifstream source(file);
    if (!source.is_open()) {
        std::cerr << "Failed to open: \t" << file << "\n";
        return;
    }
    
    std::string line;
    while (getline(source, line)) {
        
        if (!line.empty()) {
            for (char c : line) {
                if (c == '{') {
                    nest_level += 1;
                }

                if (c == '"') {
					in_string = !in_string;
                }

                if (!in_string) {
                    if (nest_level == 2 and c == ',') {
						phase++;
                    }



                }
            }
        }
    }
}


void storeAnimeDATA(int& phase, int& slot_index, std::vector<int>& slot_values) {

}


bool continuityCheck() {
    for (size_t i = 0; i < anime.size(); i++) {
        if (anime.find(i) == anime.end()) {
            std::cerr << "Anime map is missing dense ID: " << i << "\n";
            return false;
        }
    }
    return true;
}
