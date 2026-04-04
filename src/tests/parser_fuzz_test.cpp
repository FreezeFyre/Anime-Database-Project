#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../io/fileHandling.hpp"

namespace {

struct FuzzHandler {
	sectionHeader lastSection{};
	schemaData parsedSchema;
	int recordCount = 0;
	std::vector<std::string> tables;

	void section(sectionHeader& section) {
		lastSection = section;
	}

	void schema(::schemaData& incoming) {
		parsedSchema = incoming;
	}

	void record(recordData&, ::schemaData&) {
		++recordCount;
	}

	void table(std::string& value, std::string&) {
		tables.push_back(value);
	}
};

void require(bool condition, const std::string& message) {
	if (!condition) {
		throw std::runtime_error(message);
	}
}

std::string makeVectorField(std::mt19937& rng) {
	std::uniform_int_distribution<int> widthDist(0, 8);
	std::uniform_int_distribution<int> valueDist(-25, 250);

	const int width = widthDist(rng);
	std::ostringstream out;
	out << '{';
	for (int index = 0; index < width; ++index) {
		if (index > 0) {
			out << ',';
		}
		out << valueDist(rng);
	}
	out << '}';
	return out.str();
}

std::string randomTitle(std::mt19937& rng, int id) {
	std::uniform_int_distribution<int> letterDist(0, 25);
	std::uniform_int_distribution<int> lenDist(5, 24);

	std::string title = "Rec " + std::to_string(id) + " ";
	const int tailLength = lenDist(rng);
	for (int index = 0; index < tailLength; ++index) {
		title.push_back(static_cast<char>('A' + letterDist(rng)));
	}
	return title;
}

void writeRandomValidDatabase(const std::filesystem::path& path, int recordCount, uint32_t seed) {
	std::mt19937 rng(seed);
	std::uniform_int_distribution<int> scoreDist(0, 65535);
	std::uniform_int_distribution<int> groupDist(0, 255);

	std::ofstream out(path, std::ios::trunc);
	if (!out.is_open()) {
		throw std::runtime_error("Failed to create fuzz input: " + path.string());
	}

	out << "ADP0|1001|0|1|\n\n";
	out << "0|FUZZ_DATA|" << recordCount << "|5|1|\n\n";
	out << "[id:U32;N,title:STR;N,score:U16;R,tags:VECU8;MV,group:U8;MV]\n\n";

	for (int id = 0; id < recordCount; ++id) {
		out << '{'
			<< id << ','
			<< '"' << randomTitle(rng, id) << '"' << ','
			<< scoreDist(rng) << ','
			<< makeVectorField(rng) << ','
			<< groupDist(rng)
			<< "}\n";
	}
}

void writeTextFile(const std::filesystem::path& path, const std::string& content) {
	std::ofstream out(path, std::ios::trunc);
	if (!out.is_open()) {
		throw std::runtime_error("Failed to create fuzz text file: " + path.string());
	}
	out << content;
}

void expectAnyThrow(const std::filesystem::path& path, fileType expectedType, const std::string& label) {
	fileProcessor parser;
	FuzzHandler handler;
	bool threw = false;

	try {
		parser.readFile(path.string(), handler, expectedType);
	}
	catch (const std::exception&) {
		threw = true;
	}
	require(threw, label + " did not throw.");
}

void runValidRandomizedCases(const std::filesystem::path& dir) {
	fileProcessor parser;

	for (int round = 0; round < 8; ++round) {
		const int recordCount = 50 + (round * 137);
		const auto path = dir / ("valid_random_" + std::to_string(round) + ".dat");
		writeRandomValidDatabase(path, recordCount, 1337u + static_cast<uint32_t>(round));

		FuzzHandler handler;
		parser.readFile(path.string(), handler, fileType::HEADER_DATABASE);

		require(handler.recordCount == recordCount, "Randomized valid parse record count mismatch.");
		require(handler.parsedSchema.size() == 5, "Randomized valid parse schema size mismatch.");
		require(handler.lastSection.recordCount == static_cast<uint32_t>(recordCount),
			"Randomized valid parse section count mismatch.");
	}
}

void runMalformedCases(const std::filesystem::path& dir) {
	const auto wrongMagic = dir / "wrong_magic.dat";
	writeTextFile(wrongMagic,
		"BAD0|1001|0|1|\n"
		"0|BROKEN|1|5|1|\n"
		"[id:U32;N,title:STR;N,score:U16;R,tags:VECU8;MV,group:U8;MV]\n"
		"{0,\"A\",12,{1,2},4}\n");
	expectAnyThrow(wrongMagic, fileType::HEADER_DATABASE, "Wrong magic");

	const auto wrongType = dir / "wrong_type.dat";
	writeTextFile(wrongType,
		"ADP0|1001|1|1|\n"
		"1|LOOKUP|1|1|0|\n"
		"{\"value\"}\n");
	expectAnyThrow(wrongType, fileType::HEADER_DATABASE, "Wrong file type");

	const auto invalidSection = dir / "invalid_section.dat";
	writeTextFile(invalidSection,
		"ADP0|1001|0|1|\n"
		"9|BROKEN|1|5|1|\n"
		"[id:U32;N,title:STR;N,score:U16;R,tags:VECU8;MV,group:U8;MV]\n"
		"{0,\"A\",12,{1,2},4}\n");
	expectAnyThrow(invalidSection, fileType::HEADER_DATABASE, "Invalid section type");

	const auto invalidSchema = dir / "invalid_schema.dat";
	writeTextFile(invalidSchema,
		"ADP0|1001|0|1|\n"
		"0|BROKEN|1|5|1|\n"
		"[id:U32;N,title:BAD;N,score:U16;R,tags:VECU8;MV,group:U8;MV]\n"
		"{0,\"A\",12,{1,2},4}\n");
	expectAnyThrow(invalidSchema, fileType::HEADER_DATABASE, "Invalid schema");

	const auto invalidVector = dir / "invalid_vector.dat";
	writeTextFile(invalidVector,
		"ADP0|1001|0|1|\n"
		"0|BROKEN|1|5|1|\n"
		"[id:U32;N,title:STR;N,score:U16;R,tags:VECU8;MV,group:U8;MV]\n"
		"{0,\"A\",12,{1,x,2},4}\n");
	expectAnyThrow(invalidVector, fileType::HEADER_DATABASE, "Invalid vector");

	const auto nonSequentialId = dir / "non_sequential_id.dat";
	writeTextFile(nonSequentialId,
		"ADP0|1001|0|1|\n"
		"0|BROKEN|2|5|1|\n"
		"[id:U32;N,title:STR;N,score:U16;R,tags:VECU8;MV,group:U8;MV]\n"
		"{0,\"A\",12,{1,2},4}\n"
		"{2,\"B\",20,{3,4},5}\n");

	fileProcessor parser;
	FuzzHandler handler;
	parser.readFile(nonSequentialId.string(), handler, fileType::HEADER_DATABASE);
	require(handler.recordCount == 2,
		"Non-sequential ids should only fail in higher-level handlers, not the low-level parser.");
}

} // namespace

int main() {
	try {
		const std::filesystem::path outputDir =
			std::filesystem::current_path() / "parser_fuzz_artifacts";
		std::filesystem::create_directories(outputDir);

		runValidRandomizedCases(outputDir);
		runMalformedCases(outputDir);

		std::cout << "Parser fuzz test passed.\n";
		return 0;
	}
	catch (const std::exception& error) {
		std::cerr << "Parser fuzz test failed: " << error.what() << '\n';
		return 1;
	}
}
