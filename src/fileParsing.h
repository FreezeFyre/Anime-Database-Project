#pragma once

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <cctype>
#include <algorithm>
#include <limits>
#include <variant>
#include <unordered_map>
#include <array>

inline bool isDigit(char c);

struct fileHeader {
	char type;
	int recordCount;
	int fieldCount;
};

struct schemaField {
	enum class dataType {
		U8,
		U16,
		U32,
		STR,
		VECU8,
		VECU16,
		UNKOWN
	};

	std::string name;
	dataType type;
};

const char* typeToString(schemaField::dataType t);

using schemaData = std::vector<schemaField>;

using recordField = std::variant<std::string, int, std::vector<int>>;

using recordData = std::vector<recordField>;

class fileParsing {

	public:
		void parseFile(std::string file);

		void saveFile(std::string file);

	private:
		fileHeader readHeader(std::string& line);

		schemaField::dataType readDataType(std::string& dataField);

		schemaData readSchema(std::string& line);

		recordData readRecord(schemaData& schema, std::string& line);

		void storeRecord();

};

