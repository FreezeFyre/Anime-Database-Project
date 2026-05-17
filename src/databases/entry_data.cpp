//
//  Anime_data.cpp
//  Anime List
//
//  Created by Luke Raykovitz on 10/24/25.
//
#include "entry_data.hpp"
#include "../net/router.hpp"

std::unordered_map<std::string, size_t> buildFieldNames() {
	std::unordered_map<std::string, size_t> result;

	for (size_t i = 0; i < animeMeta.size(); i++) {
		result.emplace(animeMeta[i].name, i);
	}

	return result;
}

void entryDB::loadEntries(std::string file) {
	fileProcessor parser;

	try {
		parser.readFile(file, *this, fileType::HEADER_DATABASE);
	}
	catch (const std::exception& e) {
		std::cerr << "Error Loading Anime Database: " << e.what() << "\n";
	}
}

void entryDB::loadLookupTables(std::string file) {
	fileProcessor parser;

	try {
		parser.readFile(file, *this, fileType::HEADER_TABLE);
	}
	catch (const std::exception& e) {
		std::cerr << "Error Loading Anime Lookup Tables: " << e.what() << "\n";
	}
}

void entryDB::saveEntries(std::string file) {
	fileProcessor saver;
	try {
		saver.saveFile(file, *this, fileType::HEADER_DATABASE);
	}
	catch (const std::exception& e) {
		std::cerr << "Error Saving Anime Database: " << e.what() << "\n";
	}
}

void entryDB::table(std::string& table, std::string& title) {
	if      (title == "ANIME_TAG_GENRES"   ) { lookup.genres.push_back(table);   }
	else if (title == "ANIME_TAG_CHANNELS" ) { lookup.channels.push_back(table); }
	else if (title == "ANIME_TAG_AUDIOS"   ) { lookup.audios.push_back(table);   }
	else if (title == "ANIME_TAG_SUBTITLES") { lookup.subtitles.push_back(table);}
	else if (title == "ANIME_TAG_FORMATS"  ) { lookup.formats.push_back(table);  }
	else if (title == "ANIME_TAG_STATUSES" ) { lookup.statuses.push_back(table); }
	else if (title == "ANIME_TAG_MEDIAS"   ) { lookup.medias.push_back(table);   }
	else { throw std::runtime_error("Unknown Table Title: " + title); }
}

void entryDB::record(recordData& record, schemaData& schema) {

	// Check For ID, will probably change to check against a map of existing ids to ensure there arent duplicates
	int id = std::get<int>(record[0]);
	int expectedID = database.size();
	if (id != expectedID) {
		throw std::runtime_error("Non-sequential or corrupt ID in file.");
	}

	entryData anime;
	for (size_t i = 0; i < fieldMap.size(); i++) {
			if (auto mPTR = std::get_if<intPTR>(&animeMeta[fieldMap[i]].ptr)) {
				if (auto value = std::get_if<std::monostate>(&record[i])) {
					if (animeMeta[fieldMap[i]].presence == fieldPresence::REQUIRED) {
						throw std::runtime_error("Missing require data for field \"" + animeMeta[fieldMap[i]].name + "\" at index: " + std::to_string(i) +  ", ");
					}
				}
				else if (auto value = std::get_if<int>(&record[i])) {
					anime.*(*mPTR) = *value;
				}
				else {
					throw std::runtime_error("Expected int in record field " + std::to_string(i));
				}
			}
			else if (auto mPTR = std::get_if<vecPTR>(&animeMeta[fieldMap[i]].ptr)) {
				if (auto value = std::get_if<std::monostate>(&record[i])) {
					if (animeMeta[fieldMap[i]].presence == fieldPresence::REQUIRED) {
						throw std::runtime_error("Missing require data for field \"" + animeMeta[fieldMap[i]].name + "\" at index: " + std::to_string(i) + ", ");
					}
				}
				else if (auto value = std::get_if<std::vector<int>>(&record[i])) {
					anime.*(*mPTR) = *value;
				}
				else {
					throw std::runtime_error("Expected vector in record field " + std::to_string(i));
				}

			}
			else if (auto mPTR = std::get_if<strPTR>(&animeMeta[fieldMap[i]].ptr)) {
				if (auto value = std::get_if<std::monostate>(&record[i])) {
					if (animeMeta[fieldMap[i]].presence == fieldPresence::REQUIRED) {
						throw std::runtime_error("Missing require data for field \"" + animeMeta[fieldMap[i]].name + "\" at index: " + std::to_string(i) + ", ");
					}
				}
				else if (auto value = std::get_if<std::string>(&record[i])) {
					anime.*(*mPTR) = *value;
				}
				else {
					throw std::runtime_error("Expected string in record field " + std::to_string(i));
				}
			}
			else {
				throw std::runtime_error("Invalid data type in record field " + std::to_string(i));
			}
		}

	database.emplace_back(std::move(anime));
}


// Reserves the space for all records in th at database
void entryDB::section(sectionHeader& header) {

	database.reserve(header.recordCount);
}


// I need to add duplicate schema field(names), probably using unordered_set
void entryDB::schema(schemaData& schema) {
	fieldMap.clear();
	for (size_t i = 0; i < schema.size(); i++) {
		auto field = nameMap.find(schema[i].name);

		if (field != nameMap.end()) {
			size_t fieldIndex = field->second;
			
			if (animeMeta[fieldIndex].type == schema[i].type){
				fieldMap.push_back(fieldIndex);
			}
			else {
				throw std::runtime_error("Schema field: \"" + schema[i].name + "\" at index: " + std::to_string(i) + " has the incorrect data type. ");
			}
		}
		else {
			throw std::runtime_error("Schema field: \"" + schema[i].name + "\" at index: " + std::to_string(i) + " not in animeMeta. ");
		}
	}
}


std::vector<size_t> entryDB::getFilteredIDS(const indexFilters& filters) {
	std::vector<size_t> result;

	filterProcessor processor;

	result = processor.checkFilters(filters, *this);

	return result;
}


// Non Functional
std::vector<int> entryDB::getSortedIDS() {



	return {};
}

fileHeader entryDB::getHeader(fileType type) {
	fileHeader result;
	result.type = type;

	switch (type) {
		case fileType::HEADER_DATABASE:
			result.sectionCount = 1;
			break;
		case fileType::HEADER_TABLE:
			result.sectionCount = lookupCount;
			break;
		default:
			throw std::runtime_error("Unknown file type for header: " + std::to_string(static_cast<int>(type)));
	}
	return result;
}


// Currently Only Valid for Database Sections not Lookup Tables
sectionHeader entryDB::getSection(fileType type, size_t sectionIndex) {
	sectionHeader result;

	result.type = sectionType::SECTION_DATABASE_RECORDS; // Placeholder
	result.title = "ANIME_DATA";
	result.recordCount = database.size();
	result.fieldCount = animeMeta.size();
	result.hasSchema = true;

	return result;
}


// Should Function
schemaData entryDB::getSchema(fileType type, size_t sectionIndex) {
	schemaData result;
	schemaField buffer;

	for (size_t i = 0; i < animeMeta.size(); i++)
	{
		buffer.name = animeMeta[i].name;
		buffer.type = animeMeta[i].type;
		buffer.mode = animeMeta[i].mode;

		result.push_back(buffer);
	}

	return result;
}


// Needs  Work, only works for entries and dosent use file type descrimination
recordData entryDB::getRecord(fileType type, size_t sectionIndex, size_t recordIndex) {
	recordData result;

	recordField buffer;
	for (size_t i = 0; i < animeMeta.size(); i++) {
		buffer = getField(recordIndex, i);
		result.push_back(buffer);
	}

	return result;
}

recordData entryDB::getTableRecord(fileType type, size_t sectionIndex, size_t rowIndex) {
	recordData result;



	return result;
}



// Kinda want to re-write cause this uses ai code
recordField entryDB::getField(size_t id, size_t fieldIndex)
{
	recordField result;

	const entryData& record = database[id];
	const memPTR& ptr = animeMeta[fieldIndex].ptr;

	std::visit([&](auto memberPtr) {
			result = record.*memberPtr;
		}, ptr);

	return result;
}


std::vector<uint8_t> entrySerializer::payload(entryDB& db, const std::vector<size_t>& ids) {
	std::vector<uint8_t> result;
	
	std::vector<uint8_t> buffer = Section(db.getSection(fileType::HEADER_DATABASE, 0));

	result.insert(result.end(), buffer.begin(), buffer.end());

	buffer.clear();
	buffer = Schema(db.getSchema(fileType::HEADER_DATABASE, 0));

	result.insert(result.end(), buffer.begin(), buffer.end());
	buffer.clear();

	for (size_t i = 0; i < ids.size(); i++)	{
		buffer = Entry(db.getEntry(ids[i]));
		result.insert(result.end(), buffer.begin(), buffer.end());
		buffer.clear();
	}

	return result;
}


std::vector<uint8_t> entrySerializer::Section(const sectionHeader& header) {
	ByteWriter writer;

	writer.U8(static_cast<uint8_t>(header.type));
	writer.CHAR('|');
	writer.STR(header.title);
	writer.CHAR('|');
	writer.U32(header.recordCount);
	writer.CHAR('|');
	writer.U32(header.fieldCount);
	writer.CHAR('|');
	writer.U8(static_cast<uint8_t>(header.hasSchema));
	writer.CHAR('|');

	return writer.buffer;
}


std::vector<uint8_t> entrySerializer::Schema(const schemaData& schema) {
	ByteWriter writer;
	writer.CHAR('[');

	for (size_t i = 0; i < schema.size(); i++) {
		writer.STR(schema[i].name);
		writer.CHAR(':');
		schema[i].type;
		schema[i].mode;
	}



	return {};
}


std::vector<uint8_t> entrySerializer::Entry(const entryData& entry) {
	ByteWriter writer;

	// Need to replace this with dynamic serialization
	writer.U32(entry.id);
	writer.U32(entry.rank);
	writer.STR(entry.title);
	writer.STR(entry.alt_title);
	writer.U8(entry.format);
	writer.U8(entry.status);
	writer.VECU8(entry.genres);
	writer.VECU8(entry.channels);
	writer.VECU8(entry.audios);
	writer.VECU8(entry.subtitles);
	writer.U8(entry.fan_service);
	writer.U8(entry.episode_count);
	writer.U16(entry.min_runtime);
	writer.VECU16(entry.characters);
	writer.U8(entry.ip_group);
	// Now you can write writer.buffer to a file or use it as needed


	for (size_t i = 0; i < animeMeta.size(); i++) {
		// Get the correct member pointer using the data type of each field

	}


	return writer.buffer;
}

RouteResult entryRouter::route(Message& message, Context& context) {
	RouteResult result;

	MessageCodec codec;
	entrySerializer serializer;
	ByteReader reader{ message.payload };
	std::vector<uint8_t> buffer;

	switch (message.action) {
		case ACTION_TYPE::ACT_GET:
		{
			// Handle get action for entry domain
			// Convert payload to get parameters, get parameters in this case should probably be a vector of ids to get or single id 0 for all E.X. [1, 2, 3] or [0]
			
			std::vector<size_t> reqIds = reader.VECU32(); // Convert into a usable vector of ids or single id

			buffer = serializer.payload(context.entries, reqIds); // Get the payload for the requested ids and serialize it

			return { codec.construct(COM_TYPE::RESP, DOMAIN_TYPE::ENTRY, ACTION_TYPE::ACT_GET, message.id, buffer.size(), buffer), false };
			break;
		}
		case ACTION_TYPE::ACT_UPDATE:
			// Handle update action for entry domain

			break;
		case ACTION_TYPE::ACT_DELETE:
			// Handle delete action for entry domain
			break;
		case ACTION_TYPE::ACT_SEARCH:
			// Handle search action for entry domain
			break;
		default:
			std::cerr << "Unknown action type: " << static_cast<int>(message.action) << "\n";
			break;
	}
	return result;
}
