//
//main.cpp
//
//Created by Luke Raykovitz on 4/17/25.

#include <iostream>
#include <filesystem>
#include <string>
#include <cstdint>

#include "io/fileHandling.hpp"
#include "databases/entry_data.hpp"

#include "net/net_service.hpp"



int main(int argc, char* argv[]) {
	// Release / Build code 
	std::cout << "AnimeList.Backend. Build Date: " << __DATE__ << " Time: " << __TIME__ << "\n";

	bool runServer = false;
	uint16_t port = 8080;

	for (int i = 1; i < argc; i++) {
		const std::string arg = argv[i];

		if (arg == "--server") {
			runServer = true;
		}
		else if (arg == "--port") {
			if (i + 1 >= argc) {
				std::cerr << "--port requires a value\n";
				return 1;
			}

			int parsedPort = std::stoi(argv[++i]);
			if (parsedPort <= 0 || parsedPort > 65535) {
				std::cerr << "Invalid port: " << parsedPort << "\n";
				return 1;
			}
			port = static_cast<uint16_t>(parsedPort);
		}
		else {
			std::cerr << "Unknown argument: " << arg << "\n";
			return 1;
		}
	}

	entryDB entry;
	const std::filesystem::path dataDir = std::filesystem::path(PROJECT_SOURCE_DIR) / "src" / "io";

	entry.loadEntries((dataDir / "animeDatabase.1.dat").string());

	entry.loadLookupTables((dataDir / "animeLookupTables.1.dat").string());

	entry.addEntry({
		.id = 0,
		.rank = 0,
		.title = "Test",
		.alt_title = "Alt Test Title",
		.format = 0,
		.status = 0,
		.genres = {0, 1},
		.channels = {0},
		.audios = {0},
		.subtitles = {0},
		.fan_service = 0,
		.episode_count = 26,
		.min_runtime = 24,
		.characters = {0, 1, 2},
		.ip_group = 0
		});

	entry.saveEntries((dataDir / "test.1.dat").string());

	NetService net;
	if (runServer) {
		std::cout << "Starting server on port " << port << "\n";
		net.start(port);
	}

	return 0;
}
