#include "fileParsing.h"
#include "errors.h"

inline bool isDigit(char c) {
	return c >= '0' && c <= '9';
}


// ChatGPT Codex code for debug printing
const char* typeToString(schemaField::dataType t) {
	switch (t) {
	case schemaField::dataType::U8:    return "U8";
	case schemaField::dataType::U16:   return "U16";
	case schemaField::dataType::U32:   return "U32";
	case schemaField::dataType::STR:   return "STR";
	case schemaField::dataType::VECU8: return "VECU8";
	case schemaField::dataType::VECU16: return "VECU16";
	case schemaField::dataType::UNKOWN: return "UNKOWN";
	}
	return "UNKNOWNtoPrint";
}
// End of ChatGPT Codex code for debug printing




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

recordData fileParsing::readRecord(schemaData& schema, std::string& line) {
	recordData record;
	recordField fieldBuffer;

	int nestLevel = 0;
	int phase = 0;

	for (char& c : line) {
		if (c == '{') { nestLevel++; }
		if (c == '}') { nestLevel++; }
		if (c == ',' and nestLevel == 0) { phase++; }


	}

	return record;
}


void fileParsing::storeRecord(char& fileType, schemaData& schema, recordData& record) {
	if (fileType == 'A') // Anime Database
	{
		animeDB::addAnime(schema, record);
	}
	else if (fileType == 'G') // IP Group Database
	{

	}
	else if (fileType == 'C') // Character Database
	{

	}
	else if (fileType == 'I') // Input File
	{

	}
	else if (fileType == 'T') // Tag Lookup File
	{

	}

	// Return Fatal Error -- Unknown File Type //
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

