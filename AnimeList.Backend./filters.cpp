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




bool checkIncludes(Filters& f, int id) {
    return false;
}


bool checkExcludes(Filters& f, int id) {
    return false;
}


bool checkRanges(Filters& f, int id) {
    return false;
}


void filterChecks(Filters& f) {
    std::vector<int> passed_ids;
    
    for (int i = 0; i < anime.size(); i++) {
        if (checkIncludes(f, i) and checkExcludes(f, i) and checkRanges(f, i)) {
            passed_ids.push_back(i);
        }
    }
}
