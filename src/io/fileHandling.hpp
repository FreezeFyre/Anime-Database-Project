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

#include "io/file_types.hpp"

namespace anime::storage {


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
