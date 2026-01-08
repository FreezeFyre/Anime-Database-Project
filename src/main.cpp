//
//  main.cpp
//  AnimeList.Backend.
//
//  Created by Luke Raykovitz on 4/17/25.
//



// ============================================================================
// Libraries
// ============================================================================



#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <cctype>
#include <algorithm>
#include <limits>
#include <variant>
#include <climits>



#include "anime_data.hpp"
#include "client_io.hpp"
#include "filters.hpp"
#include "console.hpp"
#include "sorting.hpp"

int arrayLevel = 0;



// ============================================================================
// Functions
// ============================================================================

// Array Level
static void arrayLeveler(char c) {
    if (c == '{') arrayLevel++;
    else if (c == '}') arrayLevel--;
}

// Parse Out Vectors
static std::vector<int> parseVector(const std::string& str) {
    std::vector<int> result;
    std::string num;
    for (char c : str) {
        if (isdigit(c) or c == '-') {
            num += c;
        } else if (!num.empty()) {
            result.push_back(std::stoi(num));
            num.clear();
        }
    }
    if (!num.empty()) result.push_back(std::stoi(num));
    return result;
}


 static std::vector<std::vector<int>> parseNestedVECTOR(const std::string& str) {
    int level = 0;
    std::vector<std::vector<int>> result;
    std::string num;
    for (char c : str) {
        if (c == '{') {
            level++;
        }
        else if (c == '}') {
            level--;
        }
        else if (level == 1) {
            num += c;
        }
        else if (level == 0) {
            result.push_back(parseVector(num));
            num.clear();
        }
    }
    if (!num.empty()) result.push_back(parseVector(num));
    return result;
}





// Print Animes From Map
 static void printAnimes(const std::vector<int>& sorted_ids) {
    
    int maxNameLength = 0;
    for (const auto& pair : anime) {
        int length = static_cast<int>(pair.second.name.length());
        if (length > maxNameLength) {
            maxNameLength = length;
        }
    }
        
    
    // Print header line with adjusted width for new ID column (4 chars)
    std::cout << "----------------------------------------------------------------------------------------------------------------------------" << std::string(maxNameLength, '-') << "\n";
    
    // Print header with new "ID" column before Rank
    std::cout << "| ID  | Rank | " << std::setw(maxNameLength) << std::left << "Name" << " |            Status ID            | Fan Service Rating | Season Count | Episode Count | Average Episode Length |\n";
    
    std::cout << "----------------------------------------------------------------------------------------------------------------------------" << std::string(maxNameLength, '-') << "\n";
    

    for (int count : sorted_ids) {
        if (anime.find(count) != anime.end()) {
            animeData& a = anime[count];

                std::cout << "| "
                    << std::setw(3) << std::right << count << " | "
                    << std::setw(4) << std::left << a.rank << " | "
                    << std::setw(maxNameLength) << std::left << a.name << " | "
                    << std::setw(31) << std::right << status.at(a.status_id) << " | "
                    << std::setw(18) << std::right << a.fan_service_rating << " | "
                    << std::setw(12) << std::right << a.season_count << " | "
                    << std::setw(13) << std::right << a.episode_count << " | "
                    << std::setw(22) << std::right << a.avg_episode_length << " | ";
                
                for (int i = 0; i < a.genre_ids.size(); i++) {
                    std::cout << genre.at(a.genre_ids[i]) << ", ";
                }
                
                std::cout  << "\n";
            
        }
    }
}



// ============================================================================
// File Saving | Parsing
// ============================================================================



// Save Anime Map to File
 static void saveAnimeToJSON(const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << "\n";
        return;
    }

    outFile << "{\n";
    bool firstAnime = true;

    // Moved writeVector definition outside the loop (unchanged)
    auto writeVector = [](std::ofstream& out, const std::vector<int>& vec) {
        out << "{";
        for (size_t i = 0; i < vec.size(); ++i) {
            out << vec[i];
            if (i < vec.size() - 1) out << ",";
        }
        out << "}";
    };

    //  Changed from map-based loop to index-based loop to preserve order
    for (size_t i = 0; i < anime.size(); ++i) {  // <-- changed loop type
        const auto& a = anime[i];                // <-- access object by index

        if (!firstAnime) outFile << "\n";
        firstAnime = false;

        outFile << "  {"
                << i << "," // 🔧 Replaced 'id' with 'i' since we're using index now
                << a.rank << ","
                << a.media_type << ","
                << a.name << ","
                << a.alt_name << ",";

        writeVector(outFile, a.service_id);
        outFile << ",";

        writeVector(outFile, a.genre_ids);
        outFile << ",";

        outFile << a.status_id << ",";

        writeVector(outFile, a.audio_lang_ids);
        outFile << ",";

        writeVector(outFile, a.sub_lang_ids);
        outFile << ","
                << a.fan_service_rating << ","
                << a.season_count << ","
                << a.episode_count << ","
                << a.avg_episode_length << ","
                << a.hour_runtime << ","
                << a.min_runtime << ",";

        writeVector(outFile, a.related_show_ids);
        outFile << ",";
        
        writeVector(outFile, a.media_versions);
        outFile << "}";
    }

    outFile << "\n}";
    outFile.close();
}


// Parse Out File to Anime Map
static void parseJson(const std::string& file) {
    std::ifstream source(file);
    
    if (!source.is_open()) {
        std::cerr << "Failed to open file: " << file << "\n";
        return;
    }

    std::string line;
    while (getline(source, line)) {
        if (line.size() <= 1) {
            line.erase(0);
        }
        
        if (line.size() >= 5) {
            line.erase(line.size() - 1);
            line.erase(0,3);
        }

        
        std::vector<std::string> tokens;
        std::string token;
        bool inQuotes = 0;
        bool inVector = 0;

        if (!line.empty()) {
            for (char c : line) {
                if (c == '"') inQuotes = !inQuotes;
                else if (c == '{') inVector = true;
                else if (c == '}') inVector = false;
                
                if (c == ',' and !inQuotes and !inVector) {
                    
                    tokens.push_back(token);
                    token.clear();
                }
                else {
                    token += c;
                }
            }
            
            if (!token.empty()) tokens.push_back(token); // add last token
                        
            int id = std::stoi(tokens[0]);

            anime[id].rank              = std::stoi(tokens[1]);
            anime[id].media_type        = std::stoi(tokens[2]);
            anime[id].name              = tokens[3];
            anime[id].alt_name          = tokens[4];
            anime[id].service_id        = parseVector(tokens[5]);
            anime[id].genre_ids         = parseVector(tokens[6]);
            anime[id].status_id         = std::stoi(tokens[7]);
            anime[id].audio_lang_ids    = parseVector(tokens[8]);
            anime[id].sub_lang_ids      = parseVector(tokens[9]);
            anime[id].fan_service_rating= std::stoi(tokens[10]);
            anime[id].season_count      = std::stoi(tokens[11]);
            anime[id].episode_count     = std::stoi(tokens[12]);
            anime[id].avg_episode_length= std::stof(tokens[13]);
            anime[id].hour_runtime      = std::stoi(tokens[14]);
            anime[id].min_runtime       = std::stoi(tokens[15]);
            anime[id].related_show_ids  = parseVector(tokens[16]);
            anime[id].media_versions    = parseVector(tokens[17]);
            
        }
        
    
    }
    source.close();
    
    
    // Open file skip line 1
    // Loop through each line
    // [Id,Rank,Media_type,"Name","Alt_name",[Service_Vec],[Genre_Vec],Status,[Audio_Vec],[Sub_Vec],Fan_Service,Seasons,Ep's,Ep_Length,Hours,Mins,[Related_Vec],[Media_Vec]]
    // anime[Id] = {t.rank, t.media ,t.name, t.alt, t.services, t.genres, t.status, t.audios, t.subs, t.fan_service, t.seasons, t.ep, t.ep_length, t.hrs, t.mins, t.related, t.medias}
    // Loop through and Remove/Tokenize Strings
    // Loop Through and tokenize based off of commas
}



// ============================================================================
// Main
// ============================================================================

int main(int argc, char* argv[]) {

    // Release / Build code
	std::cout << "AnimeList.Backend. Build Date: " << __DATE__ << " Time: " << __TIME__ << "\n";

    return 0;
}
