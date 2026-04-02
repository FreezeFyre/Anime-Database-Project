#pragma once

#include <iostream>

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

class fileProcessor {
	public:
		template<typename f>
		void readFile(std::string file, f& handler, fileType& expected_type);

	private:
		bool validateMagic(std::string& magic) {
			return magic == MAGIC_REFERENCE;
		}

		bool validateVersion(uint32_t version) {
			return version == FILE_PROCESS_VERSION;
		}

		fileHeader readHeader(std::string& line);

		sectionHeader readSectionHeader(std::string& line);

		schemaField::dataType readDataType(std::string& dataField);

		schemaField::filterMode readFilterMode(std::string& filterField);

		schemaData readSchema(std::string& line, int& fieldCount);

		bool recordVector(std::string& line, std::vector<int>& vec);

		recordData readRecord(schemaData& schema, std::string& line);

};

#include "fileHandling.tpp"