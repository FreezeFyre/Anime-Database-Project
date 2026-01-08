//
//  client_io.cpp
//  Anime List
//
//  Created by Luke Raykovitz on 10/24/25.
//

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
#include <list>

#include "client_io.hpp"
#include "filters.hpp"
#include "anime_data.hpp"

bool validFilter(int n, int slot_index) {
    if (n >= 0 and n < filterSets[slot_index]->size()) {
        return true;
    }
    std::cerr << "Invalid filter value: " << n << " for slot index: " << slot_index << "\n";
    return false;
}


void parseFilters(const std::string& file) {
    int nest_level = 0;
    int phase = 0;
    int slot_index = 0;
    bool slot_collection = 0;
    std::string buffer;
    std::vector<int> slot_values;
    
    
    Filters f;
    
    
    std::ifstream source(file);
    if (!source.is_open()) {
        std::cerr << "Failed to open file: " << file << "\n";
        return;
    }
    
    std::string line;
    while (getline(source, line)) {

        for (char c : line) {
            if (c == '{') { nest_level++; }
            
            
            if (c == ',' and nest_level == 1) {
                phase++;
            }
            
            
            if (phase == 0 and c != ',' and c != '{') {
                f.ip += c;
            }
            if (phase == 1 and c != ',') {
                f.sorting = c;
            }
            if (phase == 2 or phase == 3 or phase == 4) {
                if (c == '{' and nest_level == 3) {
                    slot_collection = true;
                    buffer.clear();
                    slot_values.clear();
                }
                if (slot_collection and (isdigit(c) or c == '-')) {
                    buffer += c;
                }
                if (slot_collection and c == ',' and nest_level == 3) {
                    if (!buffer.empty()) {
                        validFilter(stoi(buffer), slot_index);
                        slot_values.push_back(stoi(buffer));
                        buffer.clear();
                    }
                }
                if (slot_collection and c == '}' and nest_level == 3) {
                    if (!buffer.empty()) {
                        validFilter(stoi(buffer), slot_index);
                        slot_values.push_back(stoi(buffer));
                        buffer.clear();
                    }
            
                    storeFilters(phase, slot_index, slot_values, f);
                    slot_index++;
                    slot_collection = false;
                }
                if (c == '}' and nest_level == 2) {
                    slot_index = 0;
                }
            }
            
            
            if (c == '}') { nest_level--; }
            
        }
    }
}
