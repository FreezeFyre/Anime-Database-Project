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



// Declare Struct For MetaData Contents and Types
struct animeDatabase {
    int rank;
    std::string name;
    std::vector<int> service_id;
    std::vector<int> genre_ids;
    int status_id;
    std::vector<int> audio_lang_ids;
    std::vector<int> sub_lang_ids;
    int fan_service_rating;
    int season_count;
    int episode_count;
    float avg_episode_length;
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
    status[2] = "Unfinished And Cancelled";
    status[3] = "Unfinished Completion Unlickely";
    status[4] = "Unfinished Limbo";
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
    anime[0] = {1, "Rascal Does Not Dream of Bunny Girl Senpai", {0}, {1, 2, 3}, 0, {0}, {0, 1}, 1, 1, 13, 26.7};
    
    anime[1] = {2, "Solo Leveling", {0}, {0, 1}, 1, {0}, {0, 1}, 0, 1, 13, 24.0};
    
    anime[2] = {3, "Horimiya", {0}, {1, 2}, 0, {0}, {0, 1}, 0, 2, 26, 24.0};
    
    anime[3] = {4, "Lycoris Recoil", {0}, {0, 1, 3}, 0, {0}, {0, 1}, 1, 1, 13, 24.5};
    
    anime[4] = {5, "Devil May Cry", {1}, {0, 4}, 0, {0}, {0, 1}, 0, 1, 12, 22.0};
    
    anime[5] = {6, "Girls Band Cry", {0}, {1, 2, 3}, 2, {0}, {0, 1}, 0, 1, 12, 22.5};
    
    anime[6] = {7, "Frieren: Beyond Journey's End", {0}, {0, 4}, 1, {0}, {0, 1}, 1, 1, 13, 25.0};
    
    anime[7] = {8, "Uzaki-Chan Wants to Hang Out", {0}, {0, 1, 3}, 1, {0}, {0, 1}, 2, 1, 12, 23.5};
    
    anime[8] = {9, "Pseudo Harem", {0}, {1, 0}, 2, {0}, {0, 1}, 3, 1, 11, 22.0};
    
    anime[9] = {10, "The Genius Prince's Guide to Raising a Nation out of Debt", {0}, {0, 4}, 0, {0}, {0, 1}, 1, 1, 12, 23.0};
    
    anime[10] = {11, "Trapped in a Dating Sim: The World of Otome Games is Tough for Mobs", {0}, {0, 1}, 0, {0}, {0, 1}, 1, 1, 12, 24.0};
    
    anime[11] = {12, "Re:Zero - Starting Life in Another World", {0}, {0, 4, 5}, 0, {0}, {0, 1}, 2, 1, 25, 24.5};
    
    anime[12] = {13, "The Misfit Of Demon King Academy", {0}, {0}, 0, {0}, {0}, 2, 2, 41, 0.0};
    
    anime[13] = {0, "", {0}, {0}, 0, {0}, {0}, 0, 0, 0, 0.0};
    
    anime[14] = {0, "", {0}, {0}, 0, {0}, {0}, 0, 0, 0, 0.0};
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
    
    
    int count = 0;
    while(count < n)
        if (anime.find(count) != anime.end()) {
            animeDatabase& a = anime[count];
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
}



