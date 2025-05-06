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
#include <cctype>



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
    std::vector<int> media_versions;
};



// Declare Unordered Maps Globally
std::unordered_map<int, std::string> service;
std::unordered_map<int, std::string> format;
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



void initializeFormat() {
    format[0] = "TV";
    format[1] = "Movie";
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
    genre[10] = "Ecchi";
    genre[11] = "Moving";
    genre[12] = "Magic";
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
        if (!firstAnime) outFile << "\n";
        firstAnime = false;

        auto writeVector = [](std::ofstream& out, const std::vector<int>& vec) {
            out << "[";
            for (size_t i = 0; i < vec.size(); ++i) {
                out << vec[i];
                if (i < vec.size() - 1) out << ",";
            }
            out << "]";
        };


        outFile << "  ["
                << id << ","
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
        outFile << "]";

    }
    outFile << "\n]";

    outFile.close();
}



std::vector<int> parseVector(const std::string& str) {
    std::vector<int> result;
    std::string num;
    for (char c : str) {
        if (isdigit(c) || c == '-') {
            num += c;
        } else if (!num.empty()) {
            result.push_back(std::stoi(num));
            num.clear();
        }
    }
    if (!num.empty()) result.push_back(std::stoi(num));
    return result;
}



void parseJson(const std::string& file) {
    std::ifstream source(file);
    

    std::string line;
    while (getline(source, line)) {
        line.erase(line.size() - 1);
        line.erase(0,3);
        
        std::vector<std::string> tokens;
        std::string token;
        bool inQuotes = 0;
        bool inVector = 0;

        if (!line.empty()) {
            for (char c : line) {
                if (c == '"') inQuotes = !inQuotes;
                else if (c == '[') inVector = true;
                else if (c == ']') inVector = false;
                
                if (c == ',' and !inQuotes and !inVector) {
                    
                    tokens.push_back(token);
                    token.clear();
                }
                else {
                    token += c;
                }
            }
            
            if (!token.empty()) tokens.push_back(token); // add last token
                        
            int id = std::stoi(tokens[0]);  // convert "123" to 123

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



void inputAnime() {
    int id;
    std::string new_anime;
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
    
    id = anime.size();
    
    std::cout << "Do you want to input a anime? Y / N |" << "\t";
    std::cin >> new_anime;
    if (new_anime == "Y" or new_anime == "y") {
        std::cout << "| Rank | Unranked = 0 |" << "\t";
        std::cin >> rank;
        std::cout << "| Media type | 0 = TV | 1 = Film |" << "\t";
        std::cin >> media_type;
        std::cout << "| Name |";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::getline(std::cin, name);
        std::cout << "| Japanese Name |" << "\t";
        std::getline(std::cin, alt_name);
        
        std::cout << "| Enter service IDs (space-separated, end with -1): ";
        int service_input;
        while (std::cin >> service_input && service_input != -1) {
            service_id.push_back(service_input);
        }
        
        std::cout << "| Enter genre IDs (space-separated, end with -1): ";
        int genre_input;
        while (std::cin >> genre_input && genre_input != -1) {
            genre_ids.push_back(genre_input);
        }

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        
        std::cout << "| Status |" << "\t";
        std::cin >> status_id;
        
        std::cout << "| Enter audio IDs (space-separated, end with -1): ";
        int audio_input;
        while (std::cin >> audio_input && audio_input != -1) {
            audio_lang_ids.push_back(audio_input);
        }
        
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::cout << "| Enter subtitle IDs (space-separated, end with -1): ";
        int sub_input;
        while (std::cin >> sub_input && sub_input != -1) {
            sub_lang_ids.push_back(sub_input);
        }
        
        std::cout << "| Fan Service | 0 - 10 |" << "\t";
        std::cin >> fan_service_rating;
        
        std::cout << "| Season Count |" << "\t";
        std::cin >> season_count;
        
        std::cout << "| Episode Count |" << "\t";
        std::cin >> episode_count;
        
        std::cout << "| Avg Episode Length |" << "\t";
        std::cin >> avg_episode_length;
        
        std::cout << "| Hour Runtime | 0 if null |" << "\t";
        std::cin >> hour_runtime;

        std::cout << "| Minute Runtime | 0 if null |" << "\t";
        std::cin >> min_runtime;
        
        std::cout << "| Enter related show IDs (space-separated, end with -1): ";
        int related_show_input;
        while (std::cin >> related_show_input && related_show_input != -1) {
            related_show_ids.push_back(related_show_input);
        }
        
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        
        std::cout << "| Enter Media Version Ids (space-separated, end with -1): ";
        int media_version_input;
        while (std::cin >> media_version_input && media_version_input != -1) {
            media_versions.push_back(media_version_input);
        }

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        anime[id] = {rank,media_type,name,alt_name,service_id,genre_ids,status_id,audio_lang_ids,sub_lang_ids,fan_service_rating,season_count,episode_count,avg_episode_length,hour_runtime,min_runtime,related_show_ids,media_versions};
    }
    else return;
}



void printAnimes() {
    
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
    
    std::vector<int> dis_services = {};
    std::vector<int> inc_services = {};
    std::vector<int> dis_genres = {};
    std::vector<int> inc_genres = {};
    std::vector<int> dis_statuses = {};
    std::vector<int> inc_statuses = {};
    std::vector<int> dis_audios = {};
    std::vector<int> inc_audios = {};
    std::vector<int> dis_subs = {};
    std::vector<int> inc_subs = {};
    
    size_t n = anime.size();
    
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
}



int main () {
    
    std::string database = "/Users/lukeraykovitz/Documents/Projects/AnimeList.Backend./AnimeList.Backend./anime_data.json";
    
    // Initialize Unordered Maps
    initializeServices();
    initializeFormat();
    initializeGenres();
    
    initializeStatus();
    
    initializeAudio();
    initializeSub();
    
    
    // Store Anime Metadata to Struct and to Unordered Map
    parseJson(database);
    
    // Print Animes From unordered Map
    printAnimes();
    
    inputAnime();
    saveAnimeToJSON(Database);
}
