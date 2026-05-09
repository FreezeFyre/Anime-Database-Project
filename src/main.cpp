//
//main.cpp
//
//Created by Luke Raykovitz on 4/17/25.

#include <iostream>

#include "io/fileHandling.hpp"
#include "databases/entry_data.hpp"

#include "net/net_service.hpp"



int main() {
	// Release / Build code 
	std::cout << "AnimeList.Backend. Build Date: " << __DATE__ << " Time: " << __TIME__ << "\n";

	entryDB entry;

	entry.loadEntries("E:/Projects/Git/Anime-Database-Project/src/io/animeDatabase.1.dat");

	entry.loadLookupTables("E:/Projects/Git/Anime-Database-Project/src/io/animeLookupTables.1.dat");

	NetService net;

	net.start(8080);

	return 0;
}
