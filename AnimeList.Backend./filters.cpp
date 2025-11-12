//
//  Filters.cpp
//  Anime List
//
//  Created by Luke Raykovitz on 10/24/25.
//

#include <unordered_set>

#include "filters.hpp"
#include "anime_data.hpp"



void storeFilters(int phase, int slot_index, const std::vector<int>& slot_values, Filters& f){
    if (phase == 2) {
        f.Sets[slot_index].include = slot_values;
    }
    if (phase == 3) {
        f.Sets[slot_index].exclude = slot_values;
    }
    if (phase == 4) {
        
        Range& range = f.Ranges[slot_index];
        
        if (slot_values.empty()) {
            range.min.reset();
            range.max.reset();
            return;
        }
        
        auto it = slot_values.begin();
        range.min = *it;
        ++it;
        
        if (it != slot_values.end()) {
            range.max = *it;
        } else {
            range.max.reset();
        }
    }
}


bool inVector(std::vector<int>& inData, int& inFilter) {
    std::unordered_set<int> mDat(inData.begin(),inData.end());
    
    if (mDat.count(inFilter)) {
        return true;
    }
    
    return false;
}

void checkIncludes(Filters& f, int id) {
    for (int i = 0; i > SK_COUNT; i++) {
        if (i == 0) {
            
        }
        else if (f.Sets[i].include.size() > 1) {
            for (int n = 0; n < f.Sets[i].include.size(); i++) {
                if (inVector(anime[id].genre_ids, f.Sets[i].include)) {
                    <#statements#>
                }
            }
        }
        else {
            
        }
    }
}


void checkExcludes(Filters& f, int id) {
    
}


void checkRanges(Filters& f, int id) {
    
}


void filterChecks(Filters& f) {
    for (int i = 0; i < anime.size(); i++) {
        
    }
}
