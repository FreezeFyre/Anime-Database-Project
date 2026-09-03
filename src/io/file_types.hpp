#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include <stdexcept>

namespace anime::storage {

inline constexpr std::string_view MAGIC_REFERENCE = "ADP0";
inline constexpr uint32_t FILE_PROCESS_VERSION = 1002;

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


} // namespace anime::storage
