#pragma once

template<typename f>
void fileProcessor::readFile(std::string file, f& handler, fileType& expected_type) {
	std::ifstream source(file);
	if (!source.is_open()) {
		std::cerr << "Failed to open: \t" << file << "\n";
		return;
	}

	fileHeader header;
	sectionHeader section;
	schemaData schema;
	recordData record;

	int lineCount = 0;
	std::string line;
	std::string buffer;

	int header_pos = 0;
	int section_count = 0;
	int record_pos = 0;
	int section_record_pos = 0;
	

	bool readHeaderLine = true;
	bool readSectionLine = false;
	bool readRecordLine = false;
	bool readSchemaLine = false;
	

	while(getline(source, line)) {
		if (line.empty()) { continue; }
		linecount++;
		if (readHeaderLine) {
			header = readHeader(line);
			readHeaderLine = false;
			readSectionLine = true;
			lastHeader = lineCount;
		}
		if (header.type != expected_type) { /*ERROR*/ }
		if (readSectionLine) {
			section_count++;
			section = readSectionHeader(line);
			if (section.hasSchema) { readSchemaLine = true; }
			else { readRecordLine = true; }
			readHeaderLine = false;
			continue;
		}
		if (readSchemaLine) {
			schema = readSchema(line, section.fieldCount);
			readRecordLine = true;
			readSchemaLine = false;
		}
		if (readRecordLine) {
			record_pos++;
			if (section.type == sectionType::SECTION_DATABASE_RECORDS) {
				record = readRecord(schema, line);
				handler.record(record, schema);
			}
			if (section.type == sectionType::SECTION_TAG_LOOKUP) {
				buffer = line;
				buffer.erase(0, 2);
				buffer.erase(buffer.size() - 2, 2);
				handler.table(buffer, section.title);
			}
		}
		if (record_pos == section.recordCount)
		{
			readSectionLine = true;
			record_pos = 0;
		}
	}
}