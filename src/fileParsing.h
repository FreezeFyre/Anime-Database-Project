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

	};

	std::string name;
	dataType type;
};

using schemaData = std::vector<schemaField>;

class fileParsing {

	public:
		void parseFile(std::string file);

		void saveFile(std::string file);

	private:
		fileHeader readHeader(std::string& line);

		schemaData readSchema(std::string& line);

};

