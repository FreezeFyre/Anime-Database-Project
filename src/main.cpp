//
//main.cpp
//
//Created by Luke Raykovitz on 4/17/25.
// Aut Viam Inveniam Aut Faciam

#include <iostream>
#include <filesystem>
#include <exception>
#include <string>

#include "io/fileHandling.hpp"
#include "app/context.hpp"

#include "net/transport/net_service.hpp"


int main(int inSize, char* input[]) {
	const char* stage = "printing build information";
	std::filesystem::path activeFile;
	try {
		// Release / Build code
		std::cout << "AnimeList.Backend. Build Date: " << __DATE__ << " Time: " << __TIME__ << "\n";

		stage = "resolving startup paths";
		const std::filesystem::path dataRoot = std::filesystem::absolute("src/io");
		std::cout << "Working directory: " << std::filesystem::current_path() << '\n';
		std::cout << "Data directory: " << dataRoot << '\n';

		stage = "parsing arguments";
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
		stage = "constructing application context";
		anime::app::Context context;
		stage = "loading entries";
		activeFile = dataRoot / "animeDatabase.crunchyroll.dat";
		context.entries.loadEntries(activeFile);
		stage = "loading lookup tables";
		activeFile = dataRoot / "animeLookupTables.1.dat";
		context.entries.loadLookupTables(activeFile);

		stage = "loading groups";
		activeFile = dataRoot / "groupDatabase.crunchyroll.dat";
		context.groups.load(activeFile);
		activeFile.clear();
		stage = "resolving group memberships";
		context.entries.resolveMemberships(context.groups);

		stage = "compiling groups";
		context.groups.compile(context.entries);

		stage = "loading users";
		activeFile = dataRoot / "userDatabase.1.dat";
		context.users.load(activeFile);
		activeFile.clear();

		stage = "constructing network service";
		anime::net::NetService net(context);
		stage = "running network service";
		net.run();

		return 0;
	} catch (const std::exception& e) {
		std::cerr << "[ERROR] Failed while " << stage;
		if (!activeFile.empty()) std::cerr << " (file: " << activeFile << ')';
		std::cerr << ": " << e.what() << '\n';
	} catch (...) {
		std::cerr << "[ERROR] Failed while " << stage;
		if (!activeFile.empty()) std::cerr << " (file: " << activeFile << ')';
		std::cerr << ": unknown exception\n";
	}
	return 1;
}

// Load Entries, Load Groups, Check entries for ip groups and build group members from that. Validate External references from entries and groups
