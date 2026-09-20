#include <cctype>
#include <cstdint>
#include <exception>
#include <locale>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include "file_types.hpp"
#include "io/fileHandling.hpp"

namespace anime::storage {

namespace {

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
} // namespace

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

// Need to add in bounds/type verification
schemaField::dataType fileProcessor::readDataType(std::string& dataField) {
	return static_cast<schemaField::dataType>(std::stoi(dataField));
}
schemaField::filterMode fileProcessor::readFilterMode(std::string& filterField) {
	return static_cast<schemaField::filterMode>(std::stoi(filterField));
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
recordField fileProcessor::parseField(schemaField::dataType type, std::optional<std::string_view> string) {
	if (string == std::nullopt) {
		return std::monostate{};
	} else if (string.value().empty()) {
		return std::string{};
	}

	switch (type) {
		case schemaField::dataType::BOOL: {
			if (string.value() == "1") {
				return true;
			} else if (string.value() == "0") {
				return false;
			} else {
				throw std::runtime_error("Parseing a bool requires a value of 0 or 1 got: " + std::string(string.value()));
			}
		}
		case schemaField::dataType::STR: {
			return std::string(string.value());
		}
		case schemaField::dataType::U8: {
			return recordField{parseUnsigned<std::uint8_t>(string.value())};
		}
		case schemaField::dataType::U16: {
			return recordField{parseUnsigned<std::uint16_t>(string.value())};
		}
		case schemaField::dataType::U32: {
			return recordField{parseUnsigned<std::uint32_t>(string.value())};
		}
		case schemaField::dataType::U64: {
			return recordField{parseUnsigned<std::uint64_t>(string.value())};
		}
		case schemaField::dataType::VECU8: {
			return recordField{parseVector<std::vector<std::uint8_t>>(string.value())};
		}
		case schemaField::dataType::VECU16: {
			return recordField{parseVector<std::vector<std::uint16_t>>(string.value())};
		}
		case schemaField::dataType::VECU32: {
			return recordField{parseVector<std::vector<std::uint32_t>>(string.value())};
		}
		case schemaField::dataType::VECU64: {
			return recordField{parseVector<std::vector<std::uint64_t>>(string.value())};
		}
		default:
			throw std::runtime_error("Unexpected or unsupported data type: " + std::to_string(static_cast<uint8_t>(type)));
	}

}
recordData fileProcessor::readRecord(schemaData& schema, std::string& line) {
	recordData record;
	size_t fieldCount = schema.size();
	record.resize(fieldCount);

	int nestLevel = 0;
	int phase = 0;
	bool inString = false;
	std::string buffer;

	for (char c : line) {
		if (c == '{' && !inString) { nestLevel++; }

		if (nestLevel == 1 and inString and c != '"') {
			buffer.push_back(c);
		}
		else if (nestLevel == 1 and !inString and c != '{' and c != ',' and c != '"' and c != '}') {
			buffer.push_back(c);
		}
		if (nestLevel >= 2)
		{
			buffer.push_back(c);
		}

		if (nestLevel == 1 and !inString and (c == ',' or c == '}')) {
			if (fieldCount <= phase) {
				throw std::runtime_error(" Input record string has more fields than expected. Expected: " + std::to_string(fieldCount) + ", Got: " + std::to_string(phase));
			}
			record[phase] = parseField(schema[phase].type, buffer);
			buffer.clear();
		}

		if (c == ',' and nestLevel == 1 and !inString) { phase++; }
		if (c == '"') { inString = !inString; }
		if (c == '}' && !inString) { nestLevel--; }
	}

	return record;
}

// All of the following needs error handling
std::string fileProcessor::printHeader(const fileHeader& header) {
	std::ostringstream out;

	out << header.magic << '|'
		<< header.version << '|'
		<< static_cast<uint64_t>(header.type) << '|'
		<< header.sectionCount << '|';

	return out.str();
}
std::string fileProcessor::printSection(const sectionHeader& section) {
	std::ostringstream out;

	out << static_cast<uint64_t>(section.type) << '|'
		<< section.title << '|'
		<< section.recordCount << '|'
		<< section.fieldCount << '|'
		<< (section.hasSchema ? 1 : 0) << '|';

	return out.str();
}

void fileProcessor::printField(std::ostream& out, bool value) {
	out << (value ? 1 : 0);
}
void fileProcessor::printField(std::ostream& out, const std::string& value) {
	out << '"' << value << '"';
}
void fileProcessor::printField(std::ostream& out, std::monostate value) {
	return;
}
void fileProcessor::printField(std::ostream& out, int value) {
	out << std::to_string(value);
}
void fileProcessor::printField(std::ostream& out, std::vector<int> value) {
	out << '{';

	if (!value.empty()) {
		out << std::to_string(value[0]);
		for (size_t i = 1; i < value.size(); i++) {
			out << ',';
			printField(out, value[i]);
		}
	}

	out << '}';
}

std::string fileProcessor::printRecord(const recordData& record, const schemaData& schema) {
	// Should add schema to variant type validation
	std::ostringstream out;
	out.imbue(std::locale::classic());

	out << '{';
	for (size_t i = 0; i < record.size(); i++) {
		if (i > 0) {
			out << ',';
		}
		std::visit([&](const auto& fieldData){
			printField(out, fieldData);
		},record[i]);
	}
	out << '}';

	return out.str();
}

std::string fileProcessor::printTableRow(const std::string& row) {
	return "{\"" + row + "\"}";
}
std::string fileProcessor::schemaDataType(const schemaField::dataType& type) {
	return std::to_string(static_cast<uint8_t>(type));
}
std::string fileProcessor::schemaFilterMode(const schemaField::filterMode& mode) {
	return std::to_string(static_cast<uint8_t>(mode));
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

} // namespace anime::storage
