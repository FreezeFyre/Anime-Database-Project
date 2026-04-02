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
#include <functional>

inline bool isDigit(char c);

static const std::string MAGIC_REFERENCE = "ADP0";
static const uint32_t FILE_PROCESS_VERSION = 1001;

enum class fileType : uint8_t {
	HEADER_DATABASE,
	HEADER_TABLE
}; // Currently Unused in Header

struct fileHeader {
	std::string magic;
	uint32_t version;
	uint8_t type;
	uint32_t sectionCount;
};


enum class sectionType : uint8_t {
	SECTION_DATABASE_RECORDS,
	SECTION_TAG_LOOKUP
};
struct sectionHeader {
	uint8_t type;
	std::string title;
	uint32_t recordCount;
	uint32_t fieldCount;
	bool hasSchema;
};


struct schemaField {
	enum class dataType {
		U8,
		U16,
		U32,
		STR,
		VECU8,
		VECU16,
		UNKNOWN
	};
	enum class filterMode {
		FM_NONE,
		FM_RANGE,
		FM_MULTI_VALUE,
		FM_RMV, // Range and Multi Value
		FM_UNKNOWN
	};
	std::string name;
	dataType type;
	filterMode mode;
};

using schemaData = std::vector<schemaField>;

using recordField = std::variant<std::monostate, std::string, int, std::vector<int>>;

using recordData = std::vector<recordField>;

class fileHandler {

	public:
		template<typename f >
		void readFile(std::string file, f& handler, fileHeader& expected_type);

		fileHeader readFileHeader(std::string file);

		template<typename f>
		void saveFile(std::string file, f& handler);

	private:
		bool checkMagic(std::string& line);

		bool checkVersion(std::string& line);

		fileHeader readHeader(std::string& line);

		sectionHeader readSection(std::string& line);

		schemaField::dataType readDataType(std::string& dataField);

		schemaField::filterMode readFilterMode(std::string& filterField);

		schemaData readSchema(std::string& line, int& fieldCount);

		bool recordVector(std::string& line, std::vector<int>& vec);

		recordData readRecord(schemaData& schema, std::string& line);

};

#include "fileHandling.tpp"

