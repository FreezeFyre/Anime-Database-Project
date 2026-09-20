#pragma once

template <typename Source>
	requires FileWriteSource<Source>
void fileProcessor::saveFile(std::string file, Source& source, fileType type) {
	std::ofstream output(file);
	if (!output.is_open()) {
		throw std::runtime_error("Failed to Open File: " + file);
		return;
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

	output.close();
}
