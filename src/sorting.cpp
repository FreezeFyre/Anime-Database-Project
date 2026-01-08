//
//  sorting.cpp
//  Anime List
//
//  Created by Luke Raykovitz on 11/25/25.
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

#include "sorting.hpp"
#include "anime_data.hpp"

std::vector<int> sorted_ids;

void animeSorting(std::vector<int>& sorted_ids) {

    for (int i = 0; i < anime.size(); ++i) {

        // Assign INT_MAX to unranked anime so they go at the bottom
        int current_rank = (anime[i].rank == 0) ? INT_MAX : anime[i].rank;
        bool inserted = false;

        for (int n = 0; n < sorted_ids.size(); ++n) {
            int other_rank = (anime[sorted_ids[n]].rank == 0) ? INT_MAX : anime[sorted_ids[n]].rank;

            if (current_rank < other_rank) {
                sorted_ids.insert(sorted_ids.begin() + n, i);
                inserted = true;
                break;
            }
        }

        if (!inserted) {
            sorted_ids.push_back(i);
        }
    }
}