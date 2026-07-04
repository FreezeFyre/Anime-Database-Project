#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../io/fileHandling.hpp"

namespace {

struct ParsedRecord {
	int id = 0;
	std::string title;
	int score = 0;
	std::vector<int> tags;
	int group = 0;
};

struct StressHandler {
	sectionHeader lastSection{};
	schemaData lastSchema;
	std::vector<ParsedRecord> records;
	std::vector<std::string> tables;

	void section(const sectionHeader& section) {
		lastSection = section;
	}

	void schema(const sectionHeader&, const schemaData& schema) {
		lastSchema = schema;
	}

	void record(const sectionHeader&, const recordData& record, const schemaData&) {
		ParsedRecord parsed;
		parsed.id = std::get<int>(record.at(0));
		parsed.title = std::get<std::string>(record.at(1));
		parsed.score = std::get<int>(record.at(2));
		parsed.tags = std::get<std::vector<int>>(record.at(3));
		parsed.group = std::get<int>(record.at(4));
		records.push_back(std::move(parsed));
	}

	void table(const sectionHeader&, const std::string& value) {
		tables.push_back(value);
	}
};

std::string makeVectorField(int seed, int width) {
	std::ostringstream out;
	out << '{';
	for (int index = 0; index < width; ++index) {
		if (index > 0) {
			out << ',';
		}
		out << ((seed + index) % 200);
	}
	out << '}';
	return out.str();
}

std::string makeDatabaseRecord(int id) {
	std::ostringstream out;
	out << '{'
		<< id << ','
		<< '"' << "Title " << id << " payload " << (id % 97) << '"' << ','
		<< (id % 5000) << ','
		<< makeVectorField(id, 3 + (id % 5)) << ','
		<< (id % 250)
		<< '}';
	return out.str();
}

void writeDatabaseFile(const std::filesystem::path& path, int recordCount) {
	std::ofstream out(path, std::ios::trunc);
	if (!out.is_open()) {
		throw std::runtime_error("Failed to create database stress file: " + path.string());
	}

	out << "ADP0|" << FILE_PROCESS_VERSION << "|0|1|\n\n";
	out << "0|STRESS_DATA|" << recordCount << "|5|1|\n\n";
	out << "[id:2;0,title:3;0,score:1;1,tags:4;2,group:0;2]\n\n";

	for (int id = 0; id < recordCount; ++id) {
		out << makeDatabaseRecord(id) << '\n';
	}
}

void writeLookupFile(const std::filesystem::path& path, int sectionCount, int recordsPerSection) {
	std::ofstream out(path, std::ios::trunc);
	if (!out.is_open()) {
		throw std::runtime_error("Failed to create lookup stress file: " + path.string());
	}

	out << "ADP0|" << FILE_PROCESS_VERSION << "|1|" << sectionCount << "|\n\n";

	for (int section = 0; section < sectionCount; ++section) {
		out << "1|LOOKUP_" << section << '|' << recordsPerSection << "|1|0|\n";
		for (int record = 0; record < recordsPerSection; ++record) {
			out << "{\"value_" << section << '_' << record << "\"}\n";
		}
		out << '\n';
	}
}

void require(bool condition, const std::string& message) {
	if (!condition) {
		throw std::runtime_error(message);
	}
}

void validateDatabaseParse(const StressHandler& handler, int expectedRecords) {
	require(static_cast<int>(handler.records.size()) == expectedRecords,
		"Database parse record count mismatch.");
	require(handler.lastSchema.size() == 5, "Database parse schema field count mismatch.");
	require(handler.lastSection.recordCount == static_cast<uint32_t>(expectedRecords),
		"Section header record count mismatch.");

	const ParsedRecord& first = handler.records.front();
	const ParsedRecord& middle = handler.records.at(expectedRecords / 2);
	const ParsedRecord& last = handler.records.back();

	require(first.id == 0, "First record id mismatch.");
	require(first.title == "Title 0 payload 0", "First record title mismatch.");
	require(first.tags.size() == 3, "First record tag count mismatch.");

	require(middle.id == expectedRecords / 2, "Middle record id mismatch.");
	require(middle.score == (expectedRecords / 2) % 5000, "Middle record score mismatch.");

	require(last.id == expectedRecords - 1, "Last record id mismatch.");
	require(last.group == (expectedRecords - 1) % 250, "Last record group mismatch.");
	require(last.tags.size() == static_cast<size_t>(3 + ((expectedRecords - 1) % 5)),
		"Last record tag count mismatch.");
}

void validateLookupParse(const StressHandler& handler, int expectedEntries) {
	require(static_cast<int>(handler.tables.size()) == expectedEntries,
		"Lookup parse table count mismatch.");
	require(handler.lastSection.type == sectionType::SECTION_TAG_LOOKUP,
		"Lookup section type mismatch.");
	require(handler.tables.front() == "value_0_0", "First lookup entry mismatch.");
	require(handler.tables.back() == "value_3_511", "Last lookup entry mismatch.");
}

} // namespace

int main() {
	try {
		constexpr int databaseRecordCount = 25000;
		constexpr int lookupSectionCount = 4;
		constexpr int lookupRecordsPerSection = 512;
		constexpr int parseRounds = 3;

		const std::filesystem::path outputDir =
			std::filesystem::current_path() / "parser_stress_artifacts";
		std::filesystem::create_directories(outputDir);

		const std::filesystem::path databasePath = outputDir / "database_stress.dat";
		const std::filesystem::path lookupPath = outputDir / "lookup_stress.dat";

		writeDatabaseFile(databasePath, databaseRecordCount);
		writeLookupFile(lookupPath, lookupSectionCount, lookupRecordsPerSection);

		fileProcessor parser;

		auto databaseStart = std::chrono::steady_clock::now();
		for (int round = 0; round < parseRounds; ++round) {
			StressHandler handler;
			parser.readFile(databasePath.string(), handler, fileType::HEADER_DATABASE);
			validateDatabaseParse(handler, databaseRecordCount);
		}
		const auto databaseElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() - databaseStart);

		auto lookupStart = std::chrono::steady_clock::now();
		for (int round = 0; round < parseRounds; ++round) {
			StressHandler handler;
			parser.readFile(lookupPath.string(), handler, fileType::HEADER_TABLE);
			validateLookupParse(handler, lookupSectionCount * lookupRecordsPerSection);
		}
		const auto lookupElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() - lookupStart);

		std::cout
			<< "Parser stress test passed.\n"
			<< "Database records per round: " << databaseRecordCount << '\n'
			<< "Lookup entries per round: " << (lookupSectionCount * lookupRecordsPerSection) << '\n'
			<< "Rounds: " << parseRounds << '\n'
			<< "Database parse time (total ms): " << databaseElapsed.count() << '\n'
			<< "Lookup parse time (total ms): " << lookupElapsed.count() << '\n';

		return 0;
	}
	catch (const std::exception& error) {
		std::cerr << "Parser stress test failed: " << error.what() << '\n';
		return 1;
	}
}
