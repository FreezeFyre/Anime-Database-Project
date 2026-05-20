//
//  Anime_data.cpp
//  Anime List
//
//  Created by Luke Raykovitz on 10/24/25.
//
#include "entry_data.hpp"
#include "../net/routing/router.hpp"

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

void entryDB::addEntry(const entryData& entry) {
	entryData result = entry;
	result.id = database.size();
	
	// Check if user has rank permissions, if yes then rank adjust, else rank = 0
	rankInsert(result.rank);
	result.rank = 0; // Default rank, can be updated later

	// Iterate over every field
	for (size_t i = 0; i < animeMeta.size(); i++) {
		
	}

	// Template for vector field validation
	for (size_t i = 0; i < result.genres.size(); i++) {
		if (result.genres[i]-1 > lookup.genres.size()) {
			throw std::runtime_error("Genre ID " + std::to_string(result.genres[i]) + " out of table bounds.");
		}
	}

	database.push_back(result);
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

bool entryDB::rankInsert(int rank) {
	if (rank == 0) {
		return false;
	}
	for (size_t i = 0; i < database.size(); i++) {
		if (database[i].rank >= rank) { database[i].rank++; }
	}
	return true;
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



std::vector<size_t> entryDB::validateSchema(const schemaData& schema) {
	std::vector<size_t> fieldIndices;
	std::unordered_set<std::string> seen;

	for (size_t i = 0; i < schema.size(); i++) {
		const schemaField& field = schema[i];

		if (!seen.insert(field.name).second) {
			std::cerr << "Duplicate schema field: \"" << field.name
				<< "\" at index: " << i << "\n";
			throw std::runtime_error("Duplicate schema field: " + field.name);
		}

		std::unordered_map<std::string, size_t>::const_iterator found = nameMap.find(field.name);

		if (found == nameMap.end()) {
			std::cerr << "Schema field: \"" << field.name
				<< "\" at index: " << i
				<< " not in animeMeta.\n";
			throw std::runtime_error("Schema field not found in animeMeta: " + field.name);
		}

		size_t metaIndex = found->second;

		if (animeMeta[metaIndex].type != field.type) {
			std::cerr << "Schema field: \"" << field.name
				<< "\" at index: " << i
				<< " has incorrect data type.\n";
			throw std::runtime_error("Schema field has incorrect data type: " + field.name);
		}

		fieldIndices.push_back(metaIndex);
	}

	for (size_t i = 0; i < animeMeta.size(); i++) {
		const fieldData& meta = animeMeta[i];

		if (!seen.contains(meta.name)) {
			if (meta.presence == fieldPresence::REQUIRED) {
				std::cerr << "Required field \"" << meta.name
					<< "\" is missing from schema.\n";
				throw std::runtime_error("Required field is missing from schema: " + meta.name);
			}
			else if (meta.presence == fieldPresence::RECOMMENDED) {
				std::clog << "Recommended field \"" << meta.name
					<< "\" is missing from schema.\n";
			}
			else {
				std::clog << "Optional field \"" << meta.name
					<< "\" is missing from schema.\n";
			}
		}
	}

	return fieldIndices;
}


// May want to re-write
// Check This later 
// 5/19/26 16:48
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
	writer.STR(header.title);
	writer.U32(header.recordCount);
	writer.U32(header.fieldCount);
	writer.U8(static_cast<uint8_t>(header.hasSchema));

	return writer.buffer;
}

entryData entrySerializer::readEntry(ByteReader& reader, std::vector<size_t>& fieldIndices) {
	entryData result;

	for (size_t i = 0; i < fieldIndices.size(); i++) {
		switch (animeMeta[fieldIndices[i]].type) {
		case schemaField::dataType::U8:
			result.*std::get<intPTR>(animeMeta[fieldIndices[i]].ptr) = reader.U8();
			break;
		case schemaField::dataType::U16:
			result.*std::get<intPTR>(animeMeta[fieldIndices[i]].ptr) = reader.U16();
			break;
		case schemaField::dataType::U32:
			result.*std::get<intPTR>(animeMeta[fieldIndices[i]].ptr) = reader.U32();
			break;
		case schemaField::dataType::STR:
			result.*std::get<strPTR>(animeMeta[fieldIndices[i]].ptr) = reader.STR();
			break;
		case schemaField::dataType::VECU8:
			result.*std::get<vecPTR>(animeMeta[fieldIndices[i]].ptr) = reader.VECU8();
			break;
		case schemaField::dataType::VECU16:
			result.*std::get<vecPTR>(animeMeta[fieldIndices[i]].ptr) = reader.VECU16();
			break;
		default:
			throw std::runtime_error("Unknown data type in schema for field: " + animeMeta[fieldIndices[i]].name);

		}
	}
	return result;
}

std::vector<uint8_t> entrySerializer::Schema(const schemaData& schema) {
	ByteWriter writer;
	writer.U32(schema.size());

	for (const auto& field : schema) {
		writer.STR(field.name);
		writer.U8(static_cast<uint8_t>(field.type));
		writer.U8(static_cast<uint8_t>(field.mode));
	}

	return writer.buffer;
}

std::vector<uint8_t> entrySerializer::Entry(const entryData& entry) {
	ByteWriter writer;

	for (size_t i = 0; i < animeMeta.size(); i++) {
		switch (animeMeta[i].type) {
		case schemaField::dataType::U8:
			writer.U8(entry.*std::get<intPTR>(animeMeta[i].ptr));
			break;
		case schemaField::dataType::U16:
			writer.U16(entry.*std::get<intPTR>(animeMeta[i].ptr));
			break;
		case schemaField::dataType::U32:
			writer.U32(entry.*std::get<intPTR>(animeMeta[i].ptr));
			break;
		case schemaField::dataType::STR:
			writer.STR(entry.*std::get<strPTR>(animeMeta[i].ptr));
			break;
		case schemaField::dataType::VECU8:
			writer.VECU8(entry.*std::get<vecPTR>(animeMeta[i].ptr));
			break;
		case schemaField::dataType::VECU16:
			writer.VECU16(entry.*std::get<vecPTR>(animeMeta[i].ptr));
			break;
		}
	}
	return writer.buffer;
}

schemaData entrySerializer::readSchema(ByteReader& reader) {
	schemaData result;
	schemaField buffer;

	size_t fieldCount = reader.U32();

	for (size_t i = 0; i < fieldCount; i++) {
		buffer.name = reader.STR();
		buffer.type = static_cast<schemaField::dataType>(reader.U8());
		buffer.mode = static_cast<schemaField::filterMode>(reader.U8());
		result.push_back(buffer);
	}

	return result;
}	

sectionHeader entrySerializer::readSection(ByteReader& reader) {
	sectionHeader header;

	header.type = static_cast<sectionType>(reader.U8());
	header.title = reader.STR();
	header.recordCount = reader.U32();
	header.fieldCount = reader.U32();
	header.hasSchema = static_cast<bool>(reader.U8());

	return header;
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

			try {
				buffer = serializer.payload(context.entries, reqIds);
			}
			catch (const std::exception& e) {
				std::cerr << "Error generating payload: " << e.what() << "\n";
				std::string errorMsg = "Error generating payload: " + std::string(e.what());
				return { codec.construct(COM_TYPE::RESP, DOMAIN_TYPE::ENTRY, ACTION_TYPE::ACT_GET, message.id, static_cast<uint32_t>(errorMsg.size()), std::vector<uint8_t>(errorMsg.begin(), errorMsg.end())), false };
			}

			return { codec.construct(COM_TYPE::RESP, DOMAIN_TYPE::ENTRY, ACTION_TYPE::ACT_GET, message.id, buffer.size(), buffer), false };
			break;
		}
		case ACTION_TYPE::ACT_UPDATE: {
			break;
		}
		case ACTION_TYPE::ACT_CREATE: {
			// Handle create action for entry domain
			ByteReader reader{ message.payload };
			schemaData schema = serializer.readSchema(reader);

			std::vector<size_t> fieldIndices;
			try {
				 fieldIndices = context.entries.validateSchema(schema);
			}
			catch (const std::exception& e) {
				std::cerr << "Schema validation failed: " << e.what() << "\n";
				std::string errorMsg = "Schema validation failed: " + std::string(e.what());
				return { codec.construct(COM_TYPE::RESP, DOMAIN_TYPE::ENTRY, ACTION_TYPE::ACT_CREATE, message.id, static_cast<uint32_t>(errorMsg.size()), std::vector<uint8_t>(errorMsg.begin(), errorMsg.end())), false };
			}

			size_t recordCount = reader.U32();

			for (size_t i = 0; i < recordCount; i++) {
				try {
					entryData newEntry = serializer.readEntry(reader, fieldIndices);
					context.entries.addEntry(newEntry);
				}
				catch (const std::exception& e) {
					std::cerr << "Error adding entry: " << e.what() << "\n";
					std::string errorMsg = "Error adding entry: " + std::string(e.what());
					return { codec.construct(COM_TYPE::RESP, DOMAIN_TYPE::ENTRY, ACTION_TYPE::ACT_CREATE, message.id, static_cast<uint32_t>(errorMsg.size()), std::vector<uint8_t>(errorMsg.begin(), errorMsg.end())), false };
				}
			}
			return { codec.construct(COM_TYPE::RESP, DOMAIN_TYPE::ENTRY, ACTION_TYPE::ACT_CREATE, message.id, 7, {'S', 'U', 'C', 'C', 'E', 'S', 'S'}) , false};
			break;
		}
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
