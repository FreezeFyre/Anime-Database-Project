#include "fileParsing.h"
#include "errors.h"

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


schemaField::dataType fileParsing::readDataType(std::string& dataField) {
	schemaField::dataType type;

	if (dataField == "U8") type = schemaField::dataType::U8;
	else if (dataField == "U16") type = schemaField::dataType::U16;
	else if (dataField == "U32") type = schemaField::dataType::U32;
	else if (dataField == "STR") type = schemaField::dataType::STR;
	else if (dataField == "VECU8") type = schemaField::dataType::VECU8;
	else if (dataField == "VECU16") type = schemaField::dataType::VECU16;
	else type = schemaField::dataType::UNKOWN;

	return type;
}


schemaData fileParsing::readSchema(std::string& line, int& fieldCount) {

	schemaData schema;
	schemaField fBuffer;
	std::string buffer;
	int phase = 0;
	bool readingName = true;
	for (char& c : line) {
		if (c == ',') {
			fBuffer.type = readDataType(buffer);
			schema.push_back(fBuffer);
			fBuffer.name.clear();
			buffer.clear();
			phase++;
		}

		if (c == ':') {
			fBuffer.name = buffer;
			buffer.clear();
		}
		if (c != ':' and c != ',')
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
bool fileParsing::recordVector(std::string& inBuffer, std::vector<int>& vec) {
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


recordData fileParsing::readRecord(schemaData& schema, std::string& line) {
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


void fileParsing::storeRecord(char& fileType, schemaData& schema, recordData& record) {
	if (fileType == 'A') { // Anime Database

	}
	else if (fileType == 'G') { // IP Group Database

	}
	else if (fileType == 'C') { // Character Database

	}
	else if (fileType == 'T') { // Tag Lookup File

	}
	else {

	}

	// Return Fatal Error -- Unknown File Type //
}


void fileParsing::debugRecordData(recordData& record) {
	std::cout << "Record Field Count: " << record.size() << "\n";


	for (int i = 0; i < record.size(); i++) {
		if (std::holds_alternative<std::monostate>(record[i])) {
			std::cout << "Monostate! \n";
		}
		else if (std::holds_alternative<std::string>(record[i])) {
			std::cout << std::get<std::string>(record[i]) << " : Type String\n";
		}
		else if (std::holds_alternative<int>(record[i])) {
			std::cout << std::get<int>(record[i]) << " : Type Int\n";

		}
		else if (std::holds_alternative<std::vector<int>>(record[i])) {
			for (int n = 0; n < std::get<std::vector<int>>(record[i]).size(); n++) {
				std::cout << std::get<std::vector<int>>(record[i])[n] << " ";
			}
			std::cout << " : Type Int Vector\n";
		}
		else {
			std::cerr << "Unknown Type in Record Data! \n";
		}
	}

}


void fileParsing::parseFile(std::string file) {
	std::ifstream source(file);
	if (!source.is_open()) {
		std::cerr << "Failed to open: \t" << file << "\n";
		return;
	}

	schemaData schema;
	fileHeader header{};

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
			schema = readSchema(line, header.fieldCount);
			continue;
		}

		if (lineCount >= 4)
		{
			if (line.empty()) {
				//Return Error -- Record Line Empty//
				continue;
			}
			if (!line.empty() and lineCount >= (4+ header.recordCount))
			{
				//Return Non-Fatal Error -- More Records Than Expected
			}
			recordData record = readRecord(schema, line);


			if (lineCount ==4)
			{
				debugRecordData(record);
			}


			storeRecord(header.type, schema, record);
		}
	}

	// Debug Printing 
	/*std::cout << header.type << "\n" << header.recordCount << "\n" << header.fieldCount << "\n";
	for (int i = 0; i < schema.size(); i++)
	{
		std::cout << schema[i].name << typeToString(schema[i].type) << "\n";
	}*/
}

void fileParsing::saveFile(std::string file) {


}

