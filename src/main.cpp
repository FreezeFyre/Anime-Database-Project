//
//main.cpp
//
//Created by Luke Raykovitz on 4/17/25.

#include <iostream>
#include <filesystem>

#include "io/fileHandling.hpp"
#include "databases/entry_data.hpp"

#include "net/net_service.hpp"



int main() {
	// Release / Build code 
	std::cout << "AnimeList.Backend. Build Date: " << __DATE__ << " Time: " << __TIME__ << "\n";

	entryDB entry;
	const std::filesystem::path dataDir = std::filesystem::path("src") / "io";

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

	return 0;
}
