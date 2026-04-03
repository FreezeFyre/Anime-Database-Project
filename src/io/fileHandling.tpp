#pragma once

/*
Parse File Templated Function is designed to parse a file that contains database data in my file format.
The function takes a input of a file path and a class object that is expected to have header, schema, and record handlers.
The function will read a file with the proper format and split it into its Header, Schema and Records
The header provides the database type, the amount of records and the amount of fields each record has.
The schema provides the name and data type for each field in a record
*/
template<typename f>
	requires FileHandler<f>
void fileProcessor::readFile(std::string file, f& handler, const fileType& expected_type) {
	std::ifstream source(file);
	if (!source.is_open()) {
		throw std::runtime_error("Failed to Open File: " + file);
		return;
	}

	fileHeader header;
	sectionHeader section;
	schemaData schema;
	recordData record;

	std::string line;
	std::string buffer;

	int section_count = 0;
	int record_pos = 0;


	bool readHeaderLine = true;
	bool readSectionLine = false;
	bool readRecordLine = false;
	bool readSchemaLine = false;


	while (getline(source, line)) {
		if (line.empty()) { continue; }
		if (readHeaderLine) {
			header = readHeader(line);
			if (header.type != expected_type) { throw std::exception("Wrong File Type In Header"); }

			readHeaderLine = false;
			readSectionLine = true;
			continue;
		}
		if (readSectionLine) {
			section_count++;
			section = readSection(line);
			handler.section(section);
			if (section.hasSchema) { readSchemaLine = true; }
			else { readRecordLine = true; }
			readSectionLine = false;
			if (section_count > header.sectionCount) {
				throw std::exception("Inaccurate Section Count in Header or Parsing Logic Error");
			}
			continue;
		}
		if (readSchemaLine) {
			schema = readSchema(line, section.fieldCount);
			handler.schema(schema);
			readRecordLine = true;
			readSchemaLine = false;
			continue;
		}
		if (readRecordLine) {
			record_pos++;
			if (section.type == sectionType::SECTION_DATABASE_RECORDS) {
				try {
					record = readRecord(schema, line);
				}
				catch (const std::exception& e) { /* Invalid Record Handling */
					throw std::runtime_error(e.what() + std::string(" In Record ") + std::to_string(record_pos) + " of Section " + section.title);
				}
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