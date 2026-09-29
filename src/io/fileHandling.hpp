#pragma once

#include <algorithm>
#include <cctype>
#include <concepts>
#include <optional>
#include <string>

#include <charconv>
#include <cstdint>
#include <ostream>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string_view>
#include <system_error>
#include <variant>
#include <vector>
#include <filesystem>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

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
		void readFile(const std::filesystem::path& file, f& handler, const fileType& expected_type);

		template<typename Source>
			requires FileWriteSource<Source>
		void saveFile(const std::filesystem::path& file, Source& source, fileType type);

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
	
		// Helpers for parsing recordField alternatives and constructing the variant from schema types.
		template <typename T> [[nodiscard]] T parseUnsigned(std::string_view string) {
			bool invalid_digit = std::any_of(string.begin(), string.end(), [&](char c) { return c < '0' || c > '9';});
			if (invalid_digit) [[unlikely]]{
				throw std::runtime_error(" Unsigned parsing expects only real integers and digits.");
			}

			T value;

			const char* first = string.data();
			const char* last = first + string.size();

			auto [ptr, error] = std::from_chars(first, last, value);

			if (error != std::errc{}) {
				throw std::runtime_error(" From chars error: " + std::make_error_code(error).message());
			}

			return value;
		}
		template <typename T> [[nodiscard]] T parseVector(std::string_view string) {
			using ElementType = typename T::value_type;
			std::string buffer;
			T vector;

			if (string.size() > 1 && string.starts_with('{') && string.ends_with('}')) {
				string.remove_prefix(1);
				string.remove_suffix(1);

				if (string.ends_with(',')) {
					throw std::runtime_error("Trailing comma in vector");
				}
			} else {
				throw std::runtime_error(" Vector parsing expects a string wrapped with {} but got: " + std::string(string));
			}


			std::for_each(string.begin(), string.end(),[&](char c) {
				if (std::isdigit(static_cast<unsigned char>(c))) {
					buffer.push_back(c);
				} else if (c == ',' && !buffer.empty()) {
					vector.push_back(parseUnsigned<ElementType>(buffer));
					buffer.clear();
				} else {
					throw std::runtime_error(" Unexpected character in unsigned vector parsing, expects only digits, commas, and {}. Got: " + std::string(1,c));
				}
			});
			if (!buffer.empty()) {
				vector.push_back(parseUnsigned<ElementType>(buffer));
			}

			return vector;
		}
		recordField parseField(schemaField::dataType type, std::optional<std::string_view> string);

		recordData readRecord(schemaData& schema, std::string& line);

		std::string printHeader(const fileHeader& header);
		std::string printSection(const sectionHeader& section);
		std::string printSchema(const schemaData& schema);

		// Overloads for printing recordField alternatives, with additional signed integer and vector overloads.
		template <std::unsigned_integral T> requires (!std::same_as<T, bool>)
		void printField(std::ostream& out, T value) {
			out << static_cast<std::uint64_t>(value);
		}
		template <std::unsigned_integral T> requires (!std::same_as<T, bool>)
		void printField(std::ostream& out, const std::vector<T>& value) {
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
		void printField(std::ostream& out, bool value);
		void printField(std::ostream& out, const std::string& value);
		void printField(std::ostream& out, std::monostate value);
		void printField(std::ostream& out, int value);
		void printField(std::ostream& out, std::vector<int> value);


		std::string printRecord(const recordData& record, const schemaData& schema);
		std::string printTableRow(const std::string& row);
		std::string schemaDataType(const schemaField::dataType& type);
		std::string schemaFilterMode(const schemaField::filterMode& mode);

};

} // namespace anime::storage

#include "io/readFile.tpp"
#include "io/saveFile.tpp"
