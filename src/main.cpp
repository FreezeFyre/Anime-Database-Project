//
//main.cpp
//
//Created by Luke Raykovitz on 4/17/25.

#include <iostream>
#include <filesystem>

#include "io/fileHandling.hpp"
#include "app/context.hpp"

#include "net/transport/net_service.hpp"


int main(int inSize, char* input[]) {
	// Release / Build code 
	std::cout << "AnimeList.Backend. Build Date: " << __DATE__ << " Time: " << __TIME__ << "\n";

	const std::filesystem::path dataRoot = std::filesystem::absolute("src/io");
	std::cout << "Working directory: " << std::filesystem::current_path() << '\n';
	std::cout << "Data directory: " << dataRoot << '\n';

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
	anime::app::Context context;
	context.entries.loadEntries(dataRoot / "animeDatabase.1.dat");
	context.entries.loadLookupTables(dataRoot / "animeLookupTables.1.dat");

	context.users.load(dataRoot / "userDatabase.1.dat");

	anime::net::NetService net(context);
	net.run();

	return 0;
}
