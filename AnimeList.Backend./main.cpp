//
//  main.cpp
//  AnimeList.Backend.
//
//  Created by Luke Raykovitz on 4/17/25.
//

#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>



// Declare Struct For MetaData Contents and Types
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
};



// Declare Unordered Maps Globally
std::unordered_map<int, std::string> service;
std::unordered_map<int, std::string> genre;
std::unordered_map<int, std::string> status;
std::unordered_map<int, std::string> audio;
std::unordered_map<int, std::string> sub;



void initializeServices() {
    service[0] = "Crunchyroll";
    service[1] = "Netflix";
    service[2] = "Hulu";
    service[3] = "Disney Plus";
}



void initializeGenres() {
    genre[0] = "Action";
    genre[1] = "Romance";
    genre[2] = "Slice of Life";
    genre[3] = "Comedey";
    genre[4] = "Drama";
    genre[5] = "Highschool";
    genre[6] = "College";
    genre[7] = "SciFi";
    genre[8] = "Modern Day";
    genre[9] = "Fantasy";
}



void initializeStatus() {
    status[0] = "Ongoing";
    status[1] = "Completed";
    status[2] = "Limbo";
    status[3] = "Cancelled";
    status[4] = "Un-Released";
}



void initializeAudio() {
    audio[0] = "Japanese";
    audio[1] = "English";
}



void initializeSub() {
    sub[0] = "English";
}



std::unordered_map<int, animeDatabase> anime;
void storeAnimeMAP() {
    
    // anime[] = {0, "", {0}, {0}, 0, {0}, {0}, 0, 0, 0, 0.0};
    anime[0] = {1,0, "Rascal Does Not Dream of Bunny Girl Senpai","", {0}, {1, 2, 3}, 0, {0}, {0, 1}, 1, 1, 13, 26.7, 0, 0, {}};
    
    anime[1] = {2,0, "Solo Leveling","", {0}, {0, 1}, 1, {0}, {0, 1}, 0, 1, 13, 24.0, 0, 0, {}};
    
    anime[2] = {3,0, "Horimiya","", {0}, {1, 2}, 0, {0}, {0, 1}, 0, 2, 26, 24.0, 0, 0, {}};
    
    anime[3] = {4,0, "Lycoris Recoil","", {0}, {0, 1, 3}, 0, {0}, {0, 1}, 1, 1, 13, 24.5, 0, 0, {}};
    
    anime[4] = {5,0, "Devil May Cry","", {1}, {0, 4}, 0, {0}, {0, 1}, 0, 1, 12, 22.0, 0, 0, {}};
    
    anime[5] = {6,0, "Girls Band Cry","", {0}, {1, 2, 3}, 2, {0}, {0, 1}, 0, 1, 12, 22.5, 0, 0, {}};
    
    anime[6] = {7,0, "Frieren: Beyond Journey's End","", {0}, {0, 4}, 1, {0}, {0, 1}, 1, 1, 13, 25.0, 0, 0, {}};
    
    anime[7] = {8,0, "Uzaki-Chan Wants to Hang Out","", {0}, {0, 1, 3}, 1, {0}, {0, 1}, 2, 1, 12, 23.5, 0, 0, {}};
    
    anime[8] = {9,0, "Pseudo Harem","", {0}, {1, 0}, 2, {0}, {0, 1}, 3, 1, 11, 22.0, 0, 0, {}};
    
    anime[9] = {10,0, "The Genius Prince's Guide to Raising a Nation out of Debt","", {0}, {0, 4}, 0, {0}, {0, 1}, 1, 1, 12, 23.0, 0, 0, {}};
    
    anime[10] = {11,0, "Trapped in a Dating Sim: The World of Otome Games is Tough for Mobs","", {0}, {0, 1}, 0, {0}, {0, 1}, 1, 1, 12, 24.0, 0, 0, {}};
    
    anime[11] = {12,0, "Re:Zero - Starting Life in Another World","", {0}, {0, 4, 5}, 0, {0}, {0, 1}, 2, 1, 25, 24.5, 0, 0, {}};
    
    anime[12] = {13,0, "The Misfit Of Demon King Academy","", {0}, {0}, 0, {0}, {0}, 2, 2, 41, 0.0, 0, 0, {}};
    
    anime[13] = {0,0, "","", {0}, {0}, 0, {0}, {0}, 0, 0, 0, 0.0, 0, 0, {}};
    
    anime[14] = {0,0, "","", {0}, {0}, 0, {0}, {0}, 0, 0, 0, 0.0, 0, 0, {}};
}



void printVectors(const std::vector<int>& vec, const std::unordered_map<int, std::string>& map) {
    for (size_t i = 0; i < vec.size(); ++i) {
            int id = vec[i];
            if (map.find(id) != map.end()) {
                std::cout << map.at(id);
                if (i != vec.size() - 1) {
                    std::cout << ", ";
                }
            }
        }
}




bool discludeVector(const std::vector<int>& x, const std::vector<int>& y) {
    if (y.empty()) return false;

    for (int value : y) {
        if (std::find(x.begin(), x.end(), value) == x.end())
            return false;
        }
    
    return true;
}



bool includeVector(const std::vector<int>& x, const std::vector<int>& y) {
    if (y.empty()) return true;

    
    for (int value : y) {
        if (std::find(x.begin(), x.end(), value) == x.end())
            return false;
    }
        return true;
}



void saveAnimeToJSON(const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << "\n";
        return;
    }

    outFile << "[\n";
    bool firstAnime = true;
    for (const auto& [id, a] : anime) {
        if (!firstAnime) outFile << ",\n";
        firstAnime = false;

        auto writeVector = [](std::ofstream& out, const std::vector<int>& vec) {
            out << "[";
            for (size_t i = 0; i < vec.size(); ++i) {
                out << vec[i];
                if (i < vec.size() - 1) out << ", ";
            }
            out << "]";
        };

        outFile << "  {\n";
        outFile << "    \"id\": " << id << ",\n";  // <- Now includes the ID
        outFile << "    \"rank\": " << a.rank << ",\n";
        outFile << "    \"media type\": " << a.media_type << ",\n";
        outFile << "    \"name\": " << std::quoted(a.name) << ",\n";
        outFile << "    \"alt_name\": " << std::quoted(a.alt_name) << ",\n";


        outFile << "    \"service_id\": ";
        writeVector(outFile, a.service_id);
        outFile << ",\n";

        outFile << "    \"genre_ids\": ";
        writeVector(outFile, a.genre_ids);
        outFile << ",\n";

        outFile << "    \"status_id\": " << a.status_id << ",\n";

        outFile << "    \"audio_lang_ids\": ";
        writeVector(outFile, a.audio_lang_ids);
        outFile << ",\n";

        outFile << "    \"sub_lang_ids\": ";
        writeVector(outFile, a.sub_lang_ids);
        outFile << ",\n";

        outFile << "    \"fan_service_rating\": " << a.fan_service_rating << ",\n";
        outFile << "    \"season_count\": " << a.season_count << ",\n";
        outFile << "    \"episode_count\": " << a.episode_count << ",\n";
        outFile << "    \"avg_episode_length\": " << a.avg_episode_length << ",\n";
        outFile << "    \"hours\": " << a.hour_runtime << ",\n";
        outFile << "    \"minutes\": " << a.min_runtime << ",\n";
        
        outFile << "    \"related ids\": ";
        writeVector(outFile, a.related_show_ids);
        outFile << "\n";
        
        outFile << "  }";
    }
    outFile << "\n]\n";

    outFile.close();
    std::cout << "Saved anime data to " << filename << "\n";
}




int main () {
    // Initialize Unordered Maps
    initializeServices();
    initializeGenres();
    
    initializeStatus();
    
    initializeAudio();
    initializeSub();
    
    
    // Store Anime Metadata to Struct and to Unordered Map
    storeAnimeMAP();
    
    size_t n = anime.size();
    
    int maxNameLength = 0;
    for (const auto& pair : anime) {
        int length = static_cast<int>(pair.second.name.length());  // cast to int
        if (length > maxNameLength) {
            maxNameLength = length;
        }
    }
    
    std::cout << "--------------------------------------------------------------------------------------------------------------------------" << std::string(maxNameLength, '-') << "\n";
    std::cout << "| Rank | " << std::setw(maxNameLength) << std::left << "Name" << " |            Status ID            | Fan Service Rating | Season Count | Episode Count | Average Episode Length |\n";
    std::cout << "--------------------------------------------------------------------------------------------------------------------------" << std::string(maxNameLength, '-') << "\n";
        

    
    std::vector<int> dis_services= {};
    std::vector<int> inc_services= {};
    std::vector<int> dis_genres= {};
    std::vector<int> inc_genres= {};
    std::vector<int> dis_statuses= {};
    std::vector<int> inc_statuses= {};
    std::vector<int> dis_audios= {};
    std::vector<int> inc_audios= {};
    std::vector<int> dis_subs= {};
    std::vector<int> inc_subs= {};
    
    
    
    int count = 0;
    while(count < n)
        if (anime.find(count) != anime.end()) {
            animeDatabase& a = anime[count];
        
            
            if (includeVector(a.service_id, inc_services) and !discludeVector(a.service_id, dis_services) and includeVector(a.genre_ids, inc_genres) and !discludeVector(a.genre_ids, dis_genres) and includeVector(a.audio_lang_ids, inc_audios) and !discludeVector(a.audio_lang_ids, dis_audios) and includeVector(a.sub_lang_ids, inc_subs) and !discludeVector(a.sub_lang_ids, dis_subs))
                std::cout << "| "
             << std::setw(4) << std::left << a.rank << " | "
             << std::setw(maxNameLength) << std::left << a.name << " | "
             << std::setw(31) << std::right << status.at(a.status_id) << " | "
             << std::setw(18) << std::right << a.fan_service_rating << " | "
             << std::setw(12) << std::right << a.season_count << " | "
             << std::setw(13) << std::right << a.episode_count << " | "
             << std::setw(22) << std::right << a.avg_episode_length << " |\n";

        count += 1;
    }
    
    saveAnimeToJSON("/Users/lukeraykovitz/Documents/Projects/AnimeList.Backend./AnimeList.Backend./anime_data.json");

}



