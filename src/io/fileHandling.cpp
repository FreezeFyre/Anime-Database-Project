#include "fileHandling.hpp"

inline bool isDigit(char c) {
	return std::isdigit(static_cast<unsigned char>(c));
}

inline static bool isNumber(std::string& str) {
	for (int i = 0; i < str.size(); i++)
	{
		if (!isDigit(str[i]) and str[i] != '-') { return false; }
	}

	return true;
}


/*-----	The Following Functions Have Partial Error Handling and Validation Checks	-----*/
fileHeader fileProcessor::readHeader(std::string& line) {
	fileHeader header;

	std::string buffer;
	std::vector<std::string> tokens; 

	for (char c : line) {
		if (c == '|') {
			tokens.push_back(buffer);
			buffer.clear();
		}
		else {
			buffer.push_back(c);
		}
	}

	if (!checkMagic(tokens[0])) { throw std::exception("Invalid File Magic"); } // Handle Wrong Magic (Versioning and File Type Identifier)
	header.magic = tokens[0];
	if (!checkVersion(tokens[1])) { throw std::exception("Depreciated File Versioning"); } // Handle Wrong Versioning
	header.version = std::stoi(tokens[1]);

	try {
		header.type = STR_TO_FTYPE(tokens[2]);
	}
	catch (const std::exception& e) { /* Invalid File Type Handling */
		throw std::runtime_error(e.what() + std::string("In File Header: ") + line);
	}

	header.sectionCount = std::stoi(tokens[3]);

	return header;
}

sectionHeader fileProcessor::readSection(const std::string& line) {
	sectionHeader section;

	std::string buffer;
	std::vector<std::string> tokens;

	for (char c : line) {
		if (c == '|') {
			tokens.push_back(buffer);
			buffer.clear();
		}
		else {
			buffer.push_back(c);
		}
	}

	try {
		section.type = STR_TO_STYPE(tokens[0]);
	}
	catch (const std::exception& e) {  /* Invalid Section Type Handling */
		throw std::runtime_error(e.what() + std::string("In Section Header \n"));
	} 

	section.title = tokens[1];
	section.recordCount = std::stoi(tokens[2]);
	section.fieldCount = std::stoi(tokens[3]);

	if (tokens[4] == "1") { section.hasSchema = true; }
	else { section.hasSchema = false; }

	return section;
}

schemaField::dataType fileProcessor::readDataType(std::string& dataField) {
	schemaField::dataType type;

	if (dataField == "U8") type = schemaField::dataType::U8;
	else if (dataField == "U16") type = schemaField::dataType::U16;
	else if (dataField == "U32") type = schemaField::dataType::U32;
	else if (dataField == "STR") type = schemaField::dataType::STR;
	else if (dataField == "VECU8") type = schemaField::dataType::VECU8;
	else if (dataField == "VECU16") type = schemaField::dataType::VECU16;
	else throw std::exception("Invalid Field Data Type");

	return type;
}

schemaField::filterMode fileProcessor::readFilterMode(std::string& filterField) {
	schemaField::filterMode mode;

	if (filterField == "N") mode = schemaField::filterMode::FM_NONE;
	else if (filterField == "R") mode = schemaField::filterMode::FM_RANGE;
	else if (filterField == "MV") mode = schemaField::filterMode::FM_MULTI_VALUE;
	else if (filterField == "RMV") mode = schemaField::filterMode::FM_RMV;
	else throw std::runtime_error("Invalid Filter Mode");

	return mode;
}

schemaData fileProcessor::readSchema(std::string& line, const int& fieldCount) {

	schemaData schema;
	schemaField fBuffer;
	std::string buffer;
	int phase = 0;
	bool readingName = true;
	for (char& c : line) {
		if (c == ',' or c == ']') {
			try {
				fBuffer.mode = readFilterMode(buffer);
			} catch (const std::runtime_error& e) { // Throw Error if filter mode is invalid
				throw std::runtime_error(e.what() + std::string("In Schema Field ") + std::to_string(phase));
			}
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
			try {
				fBuffer.type = readDataType(buffer);
			}
			catch (const std::runtime_error& e) { // Throw Error if field data type is invalid
				throw std::runtime_error(e.what() + std::string("In Schema Field ") + std::to_string(phase));
			}
			buffer.clear();
		}
		if (c != ':' and c != ',' and c != ';' and c != '[' and c!= ']')
		{
			buffer.push_back(c);
		}
		if (c == ':') {
			readingName = false;
		}
		if (c == ',') {
			readingName = true;
		}

		if (phase > fieldCount) {
			throw std::runtime_error("Too many fields in schema line! Expected " + std::to_string(fieldCount) + " fields, but got at least " + std::to_string(phase));
		}
	}

	return schema;
}

// Takes a input string of integers seperated by commas and enclosed in {} and updates the passed vector
std::vector<int> fileProcessor::recordVector(std::string& str) {
	std::string buffer;
	std::vector<int> vec;

	for (char c : str) {
		if (isdigit(c) or c == '-') {
			buffer.push_back(c);
		}
		if (c == ',' or c == '}') {
			if (!buffer.empty()) {
				vec.push_back(stoi(buffer));
				buffer.clear();
			}
		}
		if (!isdigit(c) and c != '-' and c != '{' and c != '}' and c != ',') {
			throw std::runtime_error("Invalid Character in Vector Field! Expected only digits, commas, and curly braces: " + str);
		}
	}
	return vec;
}

recordData fileProcessor::readRecord(schemaData& schema, std::string& line) {
	recordData record;
	record.resize(schema.size());
	recordField fieldBuffer;

	int nestLevel = 0;
	int phase = 0;
	bool inString = false;
	std::string buffer;

	for (char c : line) {
		if (c == '{') { nestLevel++; }

		if (nestLevel == 1 and c != '{' and c != ',' and c != '"' and c != '}') {
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
					fieldBuffer = recordVector(buffer);

					record[phase] = fieldBuffer;
					fieldBuffer = std::monostate{};
					buffer.clear();
					break;
				}
				case schemaField::dataType::VECU16: {
					fieldBuffer = recordVector(buffer);

					record[phase] = fieldBuffer;
					fieldBuffer = std::monostate{};
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
/*-----	End of Functions With Partial Error Handling and Validation Checks	-----*/




// All of the following needs error handling
std::string fileProcessor::printHeader(const fileHeader& header) {
	std::ostringstream out;

	out << header.magic << '|'
		<< header.version << '|'
		<< static_cast<int>(header.type) << '|'
		<< header.sectionCount << '|';

	return out.str();
}

std::string fileProcessor::printSection(const sectionHeader& section) {
	std::ostringstream out;

	out << static_cast<int>(section.type) << '|'
		<< section.title << '|'
		<< section.recordCount << '|'
		<< section.fieldCount << '|'
		<< (section.hasSchema ? 1 : 0) << '|';

	return out.str();
}

std::string fileProcessor::printRecord(const recordData& record, const schemaData& schema) {
	std::ostringstream out;

	out << '{';
	for (size_t i = 0; i < record.size(); i++) {
		if (i > 0) {
			out << ',';
		}

		if (std::holds_alternative<std::monostate>(record[i])) {
			continue;
		}

		switch (schema[i].type) {
		case schemaField::dataType::U8:
		case schemaField::dataType::U16:
		case schemaField::dataType::U32:
			out << std::get<int>(record[i]);
			break;
		case schemaField::dataType::STR:
			out << '"' << std::get<std::string>(record[i]) << '"';
			break;
		case schemaField::dataType::VECU8:
		case schemaField::dataType::VECU16: {
			const std::vector<int>& vec = std::get<std::vector<int>>(record[i]);
			out << '{';
			for (size_t j = 0; j < vec.size(); j++) {
				if (j > 0) {
					out << ',';
				}
				out << vec[j];
			}
			out << '}';
			break;
		}
		default:
			throw std::runtime_error("Unknown Data Type in Record Field " + std::to_string(i));
		}
	}
	out << '}';

	return out.str();
}

std::string fileProcessor::printTableRow(const std::string& row) {
	return "{\"" + row + "\"}";
}

std::string fileProcessor::schemaDataType(const schemaField::dataType& type) {
	switch (type) {
	case schemaField::dataType::U8: return "U8";
	case schemaField::dataType::U16: return "U16";
	case schemaField::dataType::U32: return "U32";
	case schemaField::dataType::STR: return "STR";
	case schemaField::dataType::VECU8: return "VECU8";
	case schemaField::dataType::VECU16: return "VECU16";
	default: throw std::runtime_error("Unknown Data Type in Schema! \n");
	}
}

std::string fileProcessor::schemaFilterMode(const schemaField::filterMode& mode) {
	switch (mode) {
	case schemaField::filterMode::FM_NONE: return "N";
	case schemaField::filterMode::FM_RANGE: return "R";
	case schemaField::filterMode::FM_MULTI_VALUE: return "MV";
	case schemaField::filterMode::FM_RMV: return "RMV";
	default: throw std::runtime_error("Unknown Filter Mode in Schema! \n");
	}
}


std::string fileProcessor::printSchema(const schemaData& schema) {
	std::string result;
	result.push_back('[');
	for (size_t i = 0; i < schema.size(); i++) {
		if (i > 0) {
			result.push_back(',');
		}
		result.append(schema[i].name);
		result.push_back(':');
		try {
			result.append(schemaDataType(schema[i].type));
		}
		catch (const std::runtime_error& e) { // Throw Error if filter mode is invalid
			throw std::runtime_error(e.what() + std::string(" When Trying to Print Schema Field ") + std::to_string(i));
		}
		result.push_back(';');
		try {
			result.append(schemaFilterMode(schema[i].mode));
		}
		catch (const std::runtime_error& e) { // Throw Error if filter mode is invalid
			throw std::runtime_error(e.what() + std::string(" When Trying to Print Schema Field ") + std::to_string(i));
		}

	}
	result.push_back(']');
	return result;
};
