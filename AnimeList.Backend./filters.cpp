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


// Logic COMPLETE
bool inVector(const std::vector<int>& inData, const std::vector<int>& inFilter) {
    std::unordered_set<int> mDat(inData.begin(),inData.end());
    
    for (int i = 0; i < inFilter.size(); i++) {
        if (mDat.count(inFilter[i])) {
            return true;
        }
    }
    return false;
}



// Logic COMPLETE -- Replace anime[id].genre_ids with anime[id].*pointer[i]
bool checkIncludes(const Filters& f, const int id) {
    for (int i = 0; i < SK_COUNT; i++) {
        if (f.Sets[i].include.empty()) continue;
        
        if (!setsVecPTR[i]) {
            continue;
        }
        
        if (!inVector(anime[id].*setsVecPTR[i], f.Sets[i].include)) {
            return false;
        }
    }
    return true;
}


// Logic COMPLETE -- Replace anime[id].genre_ids with anime[id].*pointer[i]
bool checkExcludes(const Filters& f, const int id) {
    for (int i = 0; i < SK_COUNT; i++) {
        if (f.Sets[i].exclude.empty()) continue;
        
        if (!setsVecPTR[i]) {
            continue;
        }
        
        if (inVector(anime[id].*setsVecPTR[i], f.Sets[i].exclude)) {
            return false;
        }
    }
    return true;
}


// Logic COMPLETE
bool checkRanges(const Filters& f, const int id) {
    for (int i = 0; i < RK_COUNT; i++) {
        if (!f.Ranges[i].max.has_value() and !f.Ranges[i].min.has_value()) continue;
        
        if (f.Ranges[i].min.has_value()) {
            if (*f.Ranges[i].min > anime[id].*rangeIntPTR[i]) {
                return false;
            }
        }
        
        
        if (f.Ranges[i].max.has_value()) {
            if (*f.Ranges[i].max < anime[id].*rangeIntPTR[i]) {
                return false;
            }
        }
    }
    return true;
}

// Logic COMPLETE
void filterChecks(const Filters& f) {
    std::vector<int> passed_ids;
    
    for (int i = 0; i < anime.size(); i++) {
        if (checkIncludes(f, i) and checkExcludes(f, i) and checkRanges(f, i)) {
            passed_ids.push_back(i);
        }
    }
}
