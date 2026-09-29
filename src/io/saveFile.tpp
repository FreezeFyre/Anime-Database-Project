#pragma once

#include "io/fileHandling.hpp"

namespace anime::storage {

template <typename Source>
	requires FileWriteSource<Source>
void fileProcessor::saveFile(const std::filesystem::path& file, Source& source, fileType type) {
	namespace fs = std::filesystem;

	fs::path temp = file;
	temp += ".tmp";

	std::ofstream output(temp);
	if (!output.is_open()) {
		throw std::runtime_error("Failed to Open File: " + temp.string());
	}

	fileHeader header = source.getHeader(type);
	header.magic = MAGIC_REFERENCE;
	header.version = FILE_PROCESS_VERSION;

	output << printHeader(header) << "\n";
	output << "\n";

	for (size_t i = 0; i < header.sectionCount; i++) {
		sectionHeader section = source.getSection(type, i);

		output << printSection(section) << "\n";
		output << "\n";

		if (section.hasSchema) {
			schemaData schema = source.getSchema(type, i);
			output << printSchema(schema) << "\n";
			output << "\n";
		}

		switch (section.type) {
			case sectionType::SECTION_DATABASE_RECORDS:
				for (size_t j = 0; j < section.recordCount; j++) {
					recordData record = source.getRecord(type, i, j);
					schemaData schema = source.getSchema(type, i);
					output << printRecord(record, schema) << "\n";
				}
				break;
			case sectionType::SECTION_TAG_LOOKUP:
				for (size_t j = 0; j < section.recordCount; j++) {
					recordData record = source.getTableRecord(type, i, j);
					if (record.empty() || !std::holds_alternative<std::string>(record[0])) {
						throw std::runtime_error("Table row must contain a string field.");
					}
					output << printTableRow(std::get<std::string>(record[0])) << "\n";
				}
				break;
			default:
				throw std::runtime_error("Unknown Section Type: " + std::to_string(static_cast<int>(section.type)));
		}
	}

	output.flush();
	if (!output) {
		throw std::runtime_error("Failed to write temporary file.");
	}
	output.close();
	if (!output) {
		throw std::runtime_error("Failed to close temporary file.");
	}

	HANDLE hTemp = CreateFileW(temp.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hTemp != INVALID_HANDLE_VALUE) {
		FlushFileBuffers(hTemp);
		CloseHandle(hTemp);
	} else {
		fs::remove(temp);
	}

	if (fs::exists(file)) {
		if (!ReplaceFileW(
			file.c_str(),
			temp.c_str(),
			nullptr,
			REPLACEFILE_IGNORE_ACL_ERRORS,
			nullptr,
			nullptr
		)) {
			DWORD err = GetLastError();
			std::cerr << "ReplaceFileW failed with error code: " << err << "\n";
			fs::remove(temp);
		}
	} else {
		if (!MoveFileExW(
			temp.c_str(),
			file.c_str(),
			MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH
			)) {
			DWORD err = GetLastError();
			std::cerr << "MoveFileExW failed with error code: " << err << "\n";
			fs::remove(temp);
		}
	}
}

} // namespace anime::storage
