#include "fileHandling.hpp"

inline bool isDigit(char c) {
	return std::isdigit(static_cast<unsigned char>(c));
}

inline bool isNumber(std::string& str) {
	for (int i = 0; i < str.size(); i++)
	{
		if (!isDigit(str[i]) and str[i] != '-') { return false; }
	}

	return true;
}


bool checkMagic(std::string& magic) {
	return magic == MAGIC_REFERENCE;
}

bool checkVersion(std::string& version) {
	if (!isNumber(version)) { return false; }
	return std::stoi(version) == FILE_PROCESS_VERSION;
}

fileHeader fileHandler::readHeader(std::string& line) {
	fileHeader header;

	int phase = 0;
	std::string buffer;

	for (char& c : line) {
		if (c == '|') {
			phase++;
			continue;
		}
		if (phase == 0) { buffer.push_back(c); }
		if (phase == 1 and c == '|') {
			if (!checkMagic(buffer)) { /* Throw Error Wrong File */ }
			else {
				header.magic = buffer;
				buffer.clear();
			}
		}
		if (c != '|' and phase == 1) { buffer.push_back(c); }
		if (phase == 2 and c == '|') {
			if (!checkVersion(buffer)) { /* Throw Deprechiated Versioning, Wrong File Formating Version */ }
			else {
				header.version = std::stoi(buffer);
				buffer.clear();
			}
		}
		if (isDigit(c) and phase == 2) { buffer.push_back(c); }
		if (phase == 3 and c == '|') {
			header.type = std::stoi(buffer);
			buffer.clear();
		}
		if (isDigit(c) and phase == 3)
		{
			buffer.push_back(c);
		}
		if (c == '|' and phase == 3)
		{
			header.sectionCount = std::stoi(buffer);
			buffer.clear();
		}
	}
	return header;
}

fileHeader fileHandler::readFileHeader(std::string file) {
	std::ifstream source(file);
	if (!source.is_open()) {
		std::cerr << "Failed to open: \t" << file << "\n";
		return {};
	}
	std::string line;
	getline(source, line);
	return readHeader(line);
}


schemaField::dataType fileHandler::readDataType(std::string& dataField) {
	schemaField::dataType type;

	if (dataField == "U8") type = schemaField::dataType::U8;
	else if (dataField == "U16") type = schemaField::dataType::U16;
	else if (dataField == "U32") type = schemaField::dataType::U32;
	else if (dataField == "STR") type = schemaField::dataType::STR;
	else if (dataField == "VECU8") type = schemaField::dataType::VECU8;
	else if (dataField == "VECU16") type = schemaField::dataType::VECU16;
	else type = schemaField::dataType::UNKNOWN;

	return type;
}


schemaField::filterMode fileHandler::readFilterMode(std::string& filterField) {
	schemaField::filterMode mode;

	if (filterField == "N") mode = schemaField::filterMode::FM_NONE;
	else if (filterField == "R") mode = schemaField::filterMode::FM_RANGE;
	else if (filterField == "MV") mode = schemaField::filterMode::FM_MULTI_VALUE;
	else if (filterField == "RMV") mode = schemaField::filterMode::FM_RMV;
	else mode = schemaField::filterMode::FM_UNKNOWN;

	return mode;
}


schemaData fileHandler::readSchema(std::string& line, int& fieldCount) {

	schemaData schema;
	schemaField fBuffer;
	std::string buffer;
	int phase = 0;
	bool readingName = true;
	for (char& c : line) {
		if (c == ',') {
			fBuffer.mode = readFilterMode(buffer);
			schema.push_back(fBuffer);
			fBuffer.name.clear();
			buffer.clear();
			phase++;
		}

		if (c == ':') {
			fBuffer.name = buffer;
			buffer.clear();
		}
		if (c == ';') {
			fBuffer.type = readDataType(buffer);
			buffer.clear();
		}
		if (c != ':' and c != ',' and c != ';')
		{
			buffer.push_back(c);
		}
		if (c == ':') {
			readingName = false;
		}
		if (c == ',') {
			readingName = true;
		}

		if (phase > fieldCount)
		{
			// Return Non-Fatal Error -- More Fields Than Expected//
		}
	}

	return schema;
}

// Takes a input string of integers seperated by commas and enclosed in {} and updates the passed vector
bool fileHandler::recordVector(std::string& inBuffer, std::vector<int>& vec) {
	std::string buffer;
	vec.clear();

	for (char c : inBuffer) {
		if (isdigit(c) or c == '-') {
			buffer.push_back(c);
		}
		if (c == ',' or c == '}') {
			if (!buffer.empty()) {
				vec.push_back(stoi(buffer));
				buffer.clear();
			}
		}
		else {
			// Invlaid Character
		}
	}

	return true;
}


recordData fileHandler::readRecord(schemaData& schema, std::string& line) {
	recordData record;
	record.resize(schema.size());
	recordField fieldBuffer;

	int nestLevel = 0;
	int phase = 0;
	bool inString = false;
	std::string buffer;
	std::vector<int> vecBuffer;

	for (char c : line) {
		if (c == '{') { nestLevel++; }

		if (nestLevel == 1 and c!= '{' and c != ',' and c != '"' and c != '}') {
			buffer.push_back(c);
		}
		if (nestLevel >= 2)
		{
			buffer.push_back(c);
		}

		if (nestLevel == 1 and !inString and (c == ',' or c == '}')) {
			if (buffer.empty()) { fieldBuffer = std::monostate{}; }
			else {
				switch (schema[phase].type) {

				// Integer Handling
				case schemaField::dataType::U8: {
					fieldBuffer = stoi(buffer);
					record[phase] = fieldBuffer;
					fieldBuffer = std::monostate{};
					buffer.clear();
					break;
				}
				case schemaField::dataType::U16: {
					fieldBuffer = stoi(buffer);
					record[phase] = fieldBuffer;
					fieldBuffer = std::monostate{};
					buffer.clear();
					break;
				}
				case schemaField::dataType::U32: {
					fieldBuffer = stoi(buffer);
					record[phase] = fieldBuffer;
					fieldBuffer = std::monostate{};
					buffer.clear();
					break;
				}


				// String Handling
				case schemaField::dataType::STR: {
					fieldBuffer = buffer;
					record[phase] = fieldBuffer;
					fieldBuffer = std::monostate{};
					buffer.clear();
					break;
				}


				// Vector Handling
				case schemaField::dataType::VECU8: {
					recordVector(buffer, vecBuffer);
					fieldBuffer = vecBuffer;

					record[phase] = fieldBuffer;
					fieldBuffer = std::monostate{};
					vecBuffer.clear();
					buffer.clear();
					break;
				}
				case schemaField::dataType::VECU16: {
					recordVector(buffer, vecBuffer);
					fieldBuffer = vecBuffer;

					record[phase] = fieldBuffer;
					fieldBuffer = std::monostate{};
					vecBuffer.clear();
					buffer.clear();
					break;
				}

				default:
					throw std::runtime_error("Unknown Data Type in Schema! \n");
				}
			}
		}

		if (c == ',' and nestLevel == 1 and !inString) { phase++; }
		if (c == '"') { inString = !inString; }
		if (c == '}') { nestLevel--; }
	}

	return record;
}