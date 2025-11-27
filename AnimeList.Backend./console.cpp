//
//  console.cpp
//  Anime List
//
//  Created by Luke Raykovitz on 11/24/25.
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
#include <climits>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "console.hpp"


void consoleFilterINPUT() {
    std::string input;
    std::cout << "Do you want to input filters? \t | \t y / n" << "\n";
    std::cin >> input;
    if (input == "n" or input == "N") {
        return;
    }
    
    else if (input == "y" or input == "Y") {
        input.clear();
        std::cout << "Do you have tags you want to include? \t | \t y / n" << "\n";
        std::cin >> input;
        if (input == "y" or "Y") {
            std::cout << "What Meta Data Section? \t | \t service, format, genre, status, audio, sub, media" << "\n";

        }
        
        input.clear();
        std::cout << "Do you have tags you want to exclude? \t | \t y / n" << "\n";
        std::cin >> input;
        if (input == "y" or "Y") {
            std::cout << "What Meta Data Section? \t | \t service, format, genre, status, audio, sub, media" << "\n";

        }
        
        input.clear();
        std::cout << "Do you have any ranges? \t | \t y / n" << "\n";
        std::cin >> input;
        if (input == "y" or "Y") {
            std::cout << "What Meta Data Section? \t | \t service, format, genre, status, audio, sub, media" << "\n";

        }
        
        
        std::cout << "Enter tags seperated with commas and end as the last tag \t | \t tag1,tag2,tag3,end" << "\n";
        
        std::cout << "Enter next section name or 0 for none" << "\n";
    }
}
