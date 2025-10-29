//
//  Filters.hpp
//  Anime List
//
//  Created by Luke Raykovitz on 10/24/25.
//

#pragma once

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


enum SetsKey : int {
    SK_IDS,
    SK_MEDIA_TYPE,
    SK_SERVICES,
    SK_GENRES,
    SK_STATUSES,
    SK_DUBS,
    SK_SUBS,
    SK_MEDIA_VERSIONS,
    SK_COUNT
};

enum RangeKey : int {
    RK_RANK,
    RK_FAN_SERVICE,
    RK_SEASONS,
    RK_EP_COUNT,
    RK_EP_LENGTH,
    RK_HOUR_RUN,
    RK_MIN_RUN,
    RK_COUNT
};


struct filterType {
    std::vector<int> include;
    std::vector<int> exclude;
};

struct Range {
    std::optional<int> min;
    std::optional<int> max;
};

struct Filters {
    std::string ip;
    int sorting;
    std::array<filterType, SK_COUNT> Sets;
    std::array<Range, RK_COUNT> Ranges;
};



void storeFilters(int phase, int slot_index, const std::list<int>& slot_values, Filters& f);

/* Filters_hpp */
