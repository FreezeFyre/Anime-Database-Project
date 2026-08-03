//
//main.cpp
//
//Created by Luke Raykovitz on 4/17/25.

#include <iostream>
#include <filesystem>

#include "io/fileHandling.hpp"
#include "databases/entry_data.hpp"

#include "net/transport/net_service.hpp"


int main(int inSize, char* input[]) {

	// Release / Build code 
	std::cout << "AnimeList.Backend. Build Date: " << __DATE__ << " Time: " << __TIME__ << "\n";

	bool runServer = false;

	for (int i = 1; i < inSize; i++) {
		const std::string arg = input[i];

		if (arg == "--server") {
			runServer = true;
		}
		else {
			std::cerr << "Unknown argument: " << arg << "\n";
			return 1;
		}
	}
	// End of arguments
	Context context;
	context.entries.loadEntries("E:/Projects/Git/Anime-Database-Project/src/io/animeDatabase.1.dat");
	context.entries.loadLookupTables("E:/Projects/Git/Anime-Database-Project/src/io/animeLookupTables.1.dat");

	context.users.load("E:/Projects/Git/Anime-Database-Project/src/io/userDatabase.1.dat");

	NetService net(context);
	net.start();
	std::cin.get();

	return 0;
}
