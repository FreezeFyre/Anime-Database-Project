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
#include <concepts>
#include <cstdint>

namespace anime::storage {

inline bool isDigit(char c);

static const std::string MAGIC_REFERENCE = "ADP0";
static const uint32_t FILE_PROCESS_VERSION = 1002;

enum class fileType : uint8_t {
	HEADER_DATABASE,
	HEADER_TABLE
};
inline fileType STR_TO_FTYPE(const std::string& str) {
	if (str == "0") return fileType::HEADER_DATABASE;
	else if (str == "1") return fileType::HEADER_TABLE;
	else throw std::invalid_argument("Invalid file type string: " + str);
}
struct fileHeader {
	std::string magic;
	uint32_t version;
	fileType type;
	uint32_t sectionCount;
};


enum class sectionType : uint8_t {
	SECTION_DATABASE_RECORDS,
	SECTION_TAG_LOOKUP
};
inline sectionType STR_TO_STYPE(const std::string& str) {
	if (str == "0") return sectionType::SECTION_DATABASE_RECORDS;
	else if (str == "1") return sectionType::SECTION_TAG_LOOKUP;
	else throw std::invalid_argument("Invalid section type string: " + str);
}
struct sectionHeader {
	sectionType type;
	std::string title;
	uint32_t recordCount;
	uint32_t fieldCount;
	bool hasSchema;
};

enum class dataType : uint8_t {
	U8,
	U16,
	U32,
	STR,
	VECU8,
	VECU16,
	UNKNOWN
};

struct schemaField {
	enum class dataType : uint8_t {
		U8,
		U16,
		U32,
		STR,
		VECU8,
		VECU16,
		UNKNOWN
	};
	enum class filterMode : uint8_t {
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


// Ensures Handler Class Functions
template<typename f>
concept FileReadHandler = requires(
	f h,
	sectionHeader section,
	schemaData schema,
	recordData record,
	std::string str
	) {
		{ h.section(section) } -> std::same_as<void>;
		{ h.schema(section, schema) } -> std::same_as<void>;
		{ h.record(section, record, schema) } -> std::same_as<void>;
		{ h.table(section, str) } -> std::same_as<void>;
};

template<typename f>
concept FileWriteSource = requires(
	f h,
	fileType type,
	size_t sectionIndex,
	size_t recordIndex
	) {
		{ h.getHeader(type) } -> std::same_as<fileHeader>;
		{ h.getSection(type, sectionIndex) } -> std::same_as<sectionHeader>;
		{ h.getSchema(type, sectionIndex) } -> std::same_as<schemaData>;
		{ h.getRecord(type, sectionIndex, recordIndex) } -> std::same_as<recordData>;
		{ h.getTableRecord(type, sectionIndex, recordIndex) } -> std::same_as<recordData>;
};

class fileProcessor {

	public:
		template<typename f>
			requires FileReadHandler<f>
		void readFile(std::string file, f& handler, const fileType& expected_type);

		template<typename Source>
			requires FileWriteSource<Source>
		void saveFile(std::string file, Source& source, fileType type);

	private:
		inline bool checkMagic(std::string& line) {
			if (line == MAGIC_REFERENCE) { return true; }
			else { return false; }
		}

		inline bool checkVersion(std::string& line) {
			if (line == std::to_string(FILE_PROCESS_VERSION)) { return true; }
			else { return false; }
		}

		fileHeader readHeader(std::string& line);

		sectionHeader readSection(const std::string& line);

		schemaField::dataType readDataType(std::string& dataField);

		schemaField::filterMode readFilterMode(std::string& filterField);

		schemaData readSchema(std::string& line, const int& fieldCount);

		std::vector<int> recordVector(std::string& line);

		recordData readRecord(schemaData& schema, std::string& line);

		std::string printHeader(const fileHeader& header);

		std::string printSection(const sectionHeader& section);

		std::string printSchema(const schemaData& schema);

		std::string printRecord(const recordData& record, const schemaData& schema);

		std::string printTableRow(const std::string& row);

		std::string schemaDataType(const schemaField::dataType& type);

		std::string schemaFilterMode(const schemaField::filterMode& mode);

};

#include "io/readFile.tpp"

#include "io/saveFile.tpp"

} // namespace anime::storage
