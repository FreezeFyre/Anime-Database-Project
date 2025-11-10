//
//  Filters.cpp
//  Anime List
//
//  Created by Luke Raykovitz on 10/24/25.
//

#include "filters.hpp"



void storeFilters(int phase, int slot_index, const std::list<int>& slot_values, Filters& f){
    if (phase == 2 or phase == 3) {
        for (int value : slot_values) {
            std::cout << value << "\n";
        }
    }
    else if (phase == 4) {
        
    }
}
