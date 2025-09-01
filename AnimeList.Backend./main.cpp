//
//  main.cpp
//  AnimeList.Backend.
//
//  Created by Luke Raykovitz on 4/17/25.
//



// Include Libraries
#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <cctype>
#include <algorithm>
#include <limits>



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
std::unordered_map<int, std::string> media;


std::unordered_map<size_t, animeDatabase> anime;


std::vector<int> sorted_ids;


// Services Hash
void initializeServices() {
    service[0] = "Crunchyroll";
    service[1] = "Netflix";
    service[2] = "Hulu";
    service[3] = "Disney Plus";
}



// Formats Hash
void initializeFormat() {
    format[0] = "TV";
    format[1] = "Movie";
}



// Genres Hash
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
    genre[13] = "Coming of Age";
}



// Status hash
void initializeStatus() {
    status[0] = "Ongoing";
    status[1] = "Completed";
    status[2] = "Limbo";
    status[3] = "Cancelled";
    status[4] = "Un-Released";
}



// Audios Hash
void initializeAudio() {
    audio[0] = "Japanese";
    audio[1] = "English";
    
}



// Subs Hash
void initializeSub() {
    sub[0] = "English";
}



// Medias Hash
void initializeMedias() {
    media[0] = "Manga";
    media[1] = "Light Novel";
    media[2] = "Film";
    media[3] = "TV Series";
}



// Update Database Json
void saveAnimeToJSON(const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << "\n";
        return;
    }

    outFile << "[\n";
    bool firstAnime = true;

    // Moved writeVector definition outside the loop (unchanged)
    auto writeVector = [](std::ofstream& out, const std::vector<int>& vec) {
        out << "[";
        for (size_t i = 0; i < vec.size(); ++i) {
            out << vec[i];
            if (i < vec.size() - 1) out << ",";
        }
        out << "]";
    };

    //  Changed from map-based loop to index-based loop to preserve order
    for (size_t i = 0; i < anime.size(); ++i) {  // <-- changed loop type
        const auto& a = anime[i];                // <-- access object by index

        if (!firstAnime) outFile << "\n";
        firstAnime = false;

        outFile << "  ["
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
        outFile << "]";
    }

    outFile << "\n]";
    outFile.close();
}




// Parse Out Vectors
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



// Parse Out Json to anime Map
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



// Input Processing
void inputAnime() {
    size_t id;
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
        
        std::string fast_input;
        
        std::cout << "Fast Input? Y / N |" << "\t";
        std::cin >> fast_input;
        
        if (fast_input == "Y" or fast_input == "y") {
            std::cout << "| Name |";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::getline(std::cin, name);


            name = "\"" + name + "\"";
            anime[id] = {0,0,name,"alt_name",{},{},0,{},{},0,1,0,24,0,0,{},{}};
        }
        
        else {
            std::cout << "| Rank | Unranked = 0 |" << "\t";
            std::cin >> rank;
            
            if (rank > 0) {
                for (int i = 0; i < anime.size(); i++) {
                    if (anime[i].rank >= rank) {
                        ++anime[i].rank;
                    }
                }
            }
            
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
            
            name = "\"" + name + "\"";
            alt_name = "\"" + alt_name + "\"";
            anime[id] = {rank,media_type,name,alt_name,service_id,genre_ids,status_id,audio_lang_ids,sub_lang_ids,fan_service_rating,season_count,episode_count,avg_episode_length,hour_runtime,min_runtime,related_show_ids,media_versions};
            
        }
    }
    else return;
}



// Print Animes From Map
void printAnimes(const std::vector<int>& sorted_ids) {
    
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
            animeDatabase& a = anime[count];

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
                    std::cout << genre[a.genre_ids[i]] << ", ";
                }
                
                std::cout  << "\n";
            
        }
    }
}





// Sorting Anime List By Rank
void animeSorting(std::vector<int>& sorted_ids) {
    
    for (size_t id = 0; id < anime.size(); ++id) {
        
        // Assign INT_MAX to unranked anime so they go at the bottom
        int current_rank = (anime[id].rank == 0) ? INT_MAX : anime[id].rank;
        bool inserted = false;
        
        for (size_t i = 0; i < sorted_ids.size(); ++i) {
            int other_rank = (anime[sorted_ids[i]].rank == 0) ? INT_MAX : anime[sorted_ids[i]].rank;
            
            if (current_rank < other_rank) {
                sorted_ids.insert(sorted_ids.begin() + i, id);
                inserted = true;
                break;
            }
        }
        
        if (!inserted) {
            sorted_ids.push_back(id);
        }
    }
}




void change_ranks() {
    std::string rank_change;
    size_t id;
    int new_rank;
    
    
    std::cout << "Do you want to change a animes rank? Y / N |" << "\t";
    std::cin >> rank_change;
    if (rank_change == "Y" or rank_change == "y") {
        std::cout << "What is the ID of the anime you want to change" << "\t";
        std::cin >> id;
        
        std::cout << "What is its new rank" << "\t";
        std::cin >> new_rank;
        
        for (int i = 0; i < anime.size(); i++) {
            if (anime[i].rank >= new_rank) {
                anime[i].rank += 1;
            }
        }
        
        anime[id].rank = new_rank;
        
    }
}



// Main
int main(int argc, char* argv[]) {
    std::string database = argc > 1 ? argv[1] : "/Users/lukeraykovitz/Documents/Projects/AnimeList.Backend./AnimeList.Backend./anime_data.json";
    
    // Initialize Unordered Maps
    initializeServices();
    initializeFormat();
    initializeGenres();
    
    initializeStatus();
    
    initializeAudio();
    initializeSub();
    
    
    // Store Anime Metadata to Struct and to Unordered Map
    parseJson(database);
    
    bool keepRunning = true;
    while (keepRunning) {
        sorted_ids.clear();
        animeSorting(sorted_ids);
        printAnimes(sorted_ids);
        
        inputAnime();
        change_ranks();
        
        std::string again;
        std::cout << "Would you like to continue modifying the list? Y / N | ";
        std::cin >> again;
        if (again != "Y" && again != "y") {
            keepRunning = false;
        }
    }
    
    saveAnimeToJSON(database);
    
}
