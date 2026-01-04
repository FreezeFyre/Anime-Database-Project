#include "fileParsing.h"
#include "errors.h"

inline bool isDigit(char c) {
	return c >= '0' && c <= '9';
}


fileHeader fileParsing::readHeader(std::string& line) {
	fileHeader header;

	int phase = 0;
	std::string buffer;
	std::string fieldBuffer;

	for (char& c : line) {
		if (c == '|') {
			phase++;
			continue;
		}
		if (phase == 0) { header.type = c; }
		if (isDigit(c) and phase == 1) { buffer.push_back(c); }
		if (isDigit(c) and phase == 2) { fieldBuffer.push_back(c); }
	}

	if (!buffer.empty()) header.recordCount = stoi(buffer);
	if (!fieldBuffer.empty()) header.fieldCount = stoi(fieldBuffer);

	return header;
}

schemaData fileParsing::readSchema(std::string& line) {

	schemaData schema;
	schemaField fBuffer;
	std::string buffer;
	int phase = 0;
	for (char& c : line) {
		if (c == ',') { phase++; }

	}

	return schema;
}

void fileParsing::parseFile(std::string file) {
	std::ifstream source(file);
	if (!source.is_open()) {
		std::cerr << "Failed to open: \t" << file << "\n";
		return;
	}

	schemaData schema;
	fileHeader header;

	int lineCount = 0;
	std::string line;
	while (getline(source, line)) {

		lineCount++;
		if (lineCount == 1) {
			if (line.empty()) {
				//Return Error -- Header Line Empty//
				continue;
			}
			header = readHeader(line);
			continue;
		}

		if (lineCount == 2) {
			if (line.empty()) {
				//Return Error -- Schema Line Empty//
				continue;
			}
			schema = readSchema(line);
			continue;
		}
	}

	// Debug Printing 
	std::cout << header.type << "\n" << header.recordCount << "\n" << header.fieldCount << "\n";
	for (int i = 0; i < schema.size(); i++)
	{
		std::cout << schema[i].name << "\n";
	}
}

void fileParsing::saveFile(std::string file) {


}

