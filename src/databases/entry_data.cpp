//
//  Anime_data.cpp
//  Anime List
//
//  Created by Luke Raykovitz on 10/24/25.
//
#include "entry_data.hpp"
#include "../net/routing/router.hpp"


//====================================
//				I/O
//====================================
void entryDB::loadEntries(std::string file) {
	fileProcessor parser;

	try {
		parser.readFile(file, *this, fileType::HEADER_DATABASE);
		constructIdMap(idMap);

	}
	catch (const std::exception& e) {
		std::cerr << "Error Loading Anime Database: " << e.what() << "\n";
	}

}
void entryDB::loadLookupTables(std::string file) {
	fileProcessor parser;

	try {
		parser.readFile(file, *this, fileType::HEADER_TABLE);
		constructTableIndexes(TableIndexes);
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



//====================================
//			Read Handlers
//====================================
void entryDB::section(const sectionHeader& header) {
	if (header.type == sectionType::SECTION_DATABASE_RECORDS) {
		database.reserve(header.recordCount);
		return;
	}

	if (header.type == sectionType::SECTION_TAG_LOOKUP) {
		try {
			validateTable(header.title);

			auto [it, inserted] =
				lookupTables.emplace(header.title, std::vector<std::string>{});

			if (!inserted) {
				throw std::runtime_error("Duplicate lookup table section: " + header.title);
			}

			it->second.reserve(header.recordCount);
		}
		catch (const std::exception& e) {
			throw std::runtime_error(
				"Error occurred while validating table \"" + header.title + "\": " + e.what()
			);
		}
	}
}
void entryDB::schema(const sectionHeader& section, const schemaData& schema) {
	fieldMap.clear();
	fieldMap = validateSchema(schema);
}
void entryDB::record(const sectionHeader& section, const recordData& record, const schemaData& schema) {

	// Check For ID, will probably change to check against a map of existing ids to ensure there arent duplicates
	int id = std::get<int>(record[0]);
	int expectedID = database.size();
	if (id != expectedID) {
		throw std::runtime_error("Non-sequential or corrupt ID in file.");
	}

	Entry anime;
	for (size_t i = 0; i < fieldMap.size(); i++) {
			if (auto mPTR = std::get_if<intPTR>(&EntryMeta[fieldMap[i]].ptr)) {
				if (auto value = std::get_if<std::monostate>(&record[i])) {
					if (EntryMeta[fieldMap[i]].info.presence == fieldPresence::REQUIRED) {
						throw std::runtime_error("Missing require data for field \"" + EntryMeta[fieldMap[i]].info.name + "\" at index: " + std::to_string(i) +  ", ");
					}
				}
				else if (auto value = std::get_if<int>(&record[i])) {
					anime.*(*mPTR) = *value;
				}
				else {
					throw std::runtime_error("Expected int in record field " + std::to_string(i));
				}
			}
			else if (auto mPTR = std::get_if<vecPTR>(&EntryMeta[fieldMap[i]].ptr)) {
				if (auto value = std::get_if<std::monostate>(&record[i])) {
					if (EntryMeta[fieldMap[i]].info.presence == fieldPresence::REQUIRED) {
						throw std::runtime_error("Missing require data for field \"" + EntryMeta[fieldMap[i]].info.name + "\" at index: " + std::to_string(i) + ", ");
					}
				}
				else if (auto value = std::get_if<std::vector<int>>(&record[i])) {
					anime.*(*mPTR) = *value;
				}
				else {
					throw std::runtime_error("Expected vector in record field " + std::to_string(i));
				}

			}
			else if (auto mPTR = std::get_if<strPTR>(&EntryMeta[fieldMap[i]].ptr)) {
				if (auto value = std::get_if<std::monostate>(&record[i])) {
					if (EntryMeta[fieldMap[i]].info.presence == fieldPresence::REQUIRED) {
						throw std::runtime_error("Missing require data for field \"" + EntryMeta[fieldMap[i]].info.name + "\" at index: " + std::to_string(i) + ", ");
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

	size_t index = database.size();
	database.emplace_back(std::move(anime));
	idMap.emplace(static_cast<EntryId>(database[index].id), index);
}
void entryDB::table(const sectionHeader& section, const std::string& table) {
	auto found = lookupTables.find(section.title);

	if (found == lookupTables.end()) {
		throw std::runtime_error("Lookup table was not initialized: " + section.title);
	}

	found->second.push_back(table);
}



//====================================
//			Write Sources
//====================================
fileHeader entryDB::getHeader(fileType type) {
	fileHeader result;
	result.type = type;

	switch (type) {
		case fileType::HEADER_DATABASE:
			result.sectionCount = 1;
			break;
		case fileType::HEADER_TABLE:
			result.sectionCount = lookupTables.size();
			break;
		default:
			throw std::runtime_error("Unknown file type for header: " + std::to_string(static_cast<int>(type)));
	}
	return result;
}
sectionHeader entryDB::getSection(fileType type, size_t sectionIndex) {
	sectionHeader result;

	result.type = sectionType::SECTION_DATABASE_RECORDS; // Placeholder
	result.title = "ANIME_DATA";
	result.recordCount = database.size();
	result.fieldCount = EntryMeta.size();
	result.hasSchema = true;

	return result;
}
schemaData entryDB::getSchema(fileType type, size_t sectionIndex) {
	schemaData result;
	schemaField buffer;

	for (size_t i = 0; i < EntryMeta.size(); i++) {
		buffer.name = EntryMeta[i].info.name;
		buffer.type = EntryMeta[i].info.type;
		buffer.mode = EntryMeta[i].info.mode;

		result.push_back(buffer);
	}

	return result;
}
recordData entryDB::getRecord(fileType type, size_t sectionIndex, size_t recordIndex) {
	recordData result;

	for (size_t i = 0; i < EntryMeta.size(); i++) {
		recordField buffer;

		buffer = getField(database[recordIndex].id, i);
		result.push_back(buffer);
	}

	return result;
}
recordData entryDB::getTableRecord(fileType type, size_t sectionIndex, size_t rowIndex) {
	recordData result;



	return result;
}



//====================================
//			Operations
//====================================
void entryDB::addEntry(const Entry& entry) {
	Entry result = entry;

	try {
		result.id = addId();
	}
	catch (const std::exception& e) {
		std::cerr << "Error generating ID for new entry: " << e.what() << "\n";
		throw; // Rethrow to allow caller to handle as well
	}

	// Check if user has rank permissions, if yes then rank adjust, else rank = 0
	if (result.rank > 0) {
		rankInsert(result.rank);
	}
	else {
		result.rank = 0; // Default rank, can be updated later
	}

	for (size_t i = 2; i < EntryMeta.size(); i++) {
		validateField(getFromEntry(result, i), i);
	}
	size_t index = database.size();
	database.push_back(result);
	idMap.emplace(static_cast<EntryId>(result.id), index);
}
void entryDB::updateEntry(const std::vector<EntryUpdate>& updates) {
	for (const EntryUpdate& update : updates) {
		if (update.fieldIndex >= EntryMeta.size()) {
			throw std::runtime_error("Update field out of bounds");
		}

		auto found = idMap.find(update.id);

		if (found == idMap.end()) {
			std::cerr << "Entry ID " << update.id << " not found for update.\n";
			throw std::runtime_error("Entry ID not found for update: " + std::to_string(update.id));
		}

		size_t index = found->second;
		if (update.fieldIndex == 0) {
			throw std::runtime_error("Entry ID updates are not allowed.");
		}

		if (update.fieldIndex == 1) {
			throw std::runtime_error("Rank updates are not supported yet.");
		}
		
		switch (update.op) {
			case UpdateOp::SET_FIELD: {
				try {
					setField(database[index], update.fieldIndex, update.value);
				}
				catch (const std::exception& e) {
					std::cerr << "Error Setting Entry" << update.id << "Field: " << update.fieldIndex << ": " << e.what() << ".\n";
					throw; // Rethrow to allow caller to handle as well
				}
				break;
			}
			case UpdateOp::CLEAR_FIELD: {
				try {
					clearField(database[index], update.fieldIndex);
				}
				catch (const std::exception& e) {
					std::cerr << "Error Clearing Entry" << update.id << "Field: " << update.fieldIndex << ": " << e.what() << ".\n";
					throw; // Rethrow to allow caller to handle as well
				}
				break;
			}
			case UpdateOp::ADD_VALUE: {
				if (EntryMeta[update.fieldIndex].info.type != schemaField::dataType::VECU8 and EntryMeta[update.fieldIndex].info.type != schemaField::dataType::VECU16) {
					throw std::runtime_error("Add value only works with vectors");
				}
				try {
					addFieldValue(database[index], update.fieldIndex, update.value);
				}
				catch (const std::exception& e) {
					std::cerr << "Error adding value to Entry" << update.id << "Field: " << e.what() << ".\n";
					throw; // Rethrow to allow caller to handle as well
				}
				break;
			}
			case UpdateOp::REMOVE_VALUE: {
				if (EntryMeta[update.fieldIndex].info.type != schemaField::dataType::VECU8 and EntryMeta[update.fieldIndex].info.type != schemaField::dataType::VECU16) {
					throw std::runtime_error("Remove value only works with vectors");
				}
				try {
					removeFieldValue(database[index], update.fieldIndex, update.value);
				}
				catch (const std::exception& e) {
					std::cerr << "Error Removing value from entry" << update.id << "Field: " << update.fieldIndex << ": " << e.what() << ".\n";
					throw; // Rethrow to allow caller to handle as well
				}
				break;
			}
			default:
				throw std::runtime_error("Unknown update operation.");
		}
	}
}
std::vector<Entry> entryDB::getEntry(std::vector<EntryId> ids) {
	if (ids.size() == 1 and ids[0] == 0) { return database; }

	std::vector<Entry> result;
	for (EntryId id : ids) {
		if (auto it = idMap.find(id); it != idMap.end()) {
			result.push_back(database[it->second]);
		}
		else {
			std::cerr << "Entry ID " << id << " not found in database.\n";
			throw std::runtime_error("Entry ID not found: " + std::to_string(id));
		}
	}
	return result;
}
void entryDB::deleteEntry(const EntryId id) {
	//check user perm level
	bool admin = false;
	if (!admin) {
		throw std::runtime_error("User does not meet account permission level to delete entries. \n");
	}

	if (idMap.find(id) != idMap.end()) {
		size_t index = idMap.find(id)->second;
		// Rank Re-Ordering
		if (database[index].rank != 0) {
			for (auto& entry : database) {
				if (entry.rank > database[index].rank) {
					entry.rank--;
				}
			}
		}

		database.erase(database.begin() + index); // may want to use swap and popback or erase_if instead
		constructIdMap(idMap);
	

		std::clog << "Erased entry " << id << " from the database. \n";
	}
}
std::vector<EntryId> entryDB::searchEntryTitles(const std::string& term) {
	std::vector<EntryId> result;
	std::string nTerm = normalizeString(term);
	
	if (nTerm.empty()) {
		return result;
	}

	for (const Entry& entry : database) {
		// Should eventually make this search all string fields in EntryMeta that have a qualifer like "SEARCHABLE"
		if (normalizeString(entry.title).find(nTerm) != std::string::npos) {
			result.push_back(entry.id);
			continue;
		}
		if (normalizeString(entry.alt_title).find(nTerm) != std::string::npos) {
			result.push_back(entry.id);
			continue;
		}
	}

	return result;
}



//====================================
//			Operation Utilities
//====================================
void entryDB::setField(Entry& entry, size_t fieldIndex, const recordField& value) {
	// Type Dynamic function
	try {
		validateField(value, fieldIndex);
	}
	catch (const std::exception& e) {
		std::cerr << "Error validating field \"" << EntryMeta[fieldIndex].info.name
			<< "\" for entry ID " << entry.id << ": " << e.what() << "\n";
		throw; // Rethrow to allow caller to handle as well
	}

	switch (EntryMeta[fieldIndex].info.type) {
		case schemaField::dataType::U8: {
			entry.*std::get<intPTR>(EntryMeta[fieldIndex].ptr) = std::get<int>(value);
			break;
		}
		case schemaField::dataType::U16: {
			entry.*std::get<intPTR>(EntryMeta[fieldIndex].ptr) = std::get<int>(value);
			break;
		}
		case schemaField::dataType::U32: {
			entry.*std::get<intPTR>(EntryMeta[fieldIndex].ptr) = std::get<int>(value);
			break;
		}
		case schemaField::dataType::STR: {
			entry.*std::get<strPTR>(EntryMeta[fieldIndex].ptr) = std::get<std::string>(value);
			break;
		}
		case schemaField::dataType::VECU8: {
			entry.*std::get<vecPTR>(EntryMeta[fieldIndex].ptr) = std::get<std::vector<int>>(value);
			break;
		}
		case schemaField::dataType::VECU16: {
			entry.*std::get<vecPTR>(EntryMeta[fieldIndex].ptr) = std::get<std::vector<int>>(value);
			break;
		}
		default:
			throw std::runtime_error("Unsupported data type for set field operation: " + EntryMeta[fieldIndex].info.name);
	}

}
// Can probably re-write to convert scalar inputs to vectors
void entryDB::addFieldValue(Entry& entry, size_t fieldIndex, const recordField& value) {
	// This function only concerns vectors
	if (!std::holds_alternative<vecPTR>(EntryMeta[fieldIndex].ptr)) {
		throw std::runtime_error("Add value only works with vector fields: " + EntryMeta[fieldIndex].info.name);
	}

	if (!std::holds_alternative<std::vector<int>>(value)) {
		throw std::runtime_error("Add field value expects a Vector of integers");
	}

	try {
		validateField(value, fieldIndex);
	}
	catch (const std::exception& e) {
		std::cerr << "Error validating field \"" << EntryMeta[fieldIndex].info.name
			<< "\" for entry ID " << entry.id << ": " << e.what() << "\n";
		throw; // Rethrow to allow caller to handle as well
	}
	
	auto& field = entry.*std::get<vecPTR>(EntryMeta[fieldIndex].ptr);
	for (int addVal : std::get<std::vector<int>>(value)) {
		field.push_back(addVal);
		std::clog << "Added value " << addVal << " to field \"" << EntryMeta[fieldIndex].info.name
			<< "\" for entry ID " << entry.id << ".\n";
	}

	try {
		validateField(field, fieldIndex);
	}
	catch (const std::exception& e) {
		std::cerr << "Error validating field \"" << EntryMeta[fieldIndex].info.name
			<< "\" for entry ID " << entry.id << ": " << e.what() << "\n";
		throw; // Rethrow to allow caller to handle as well
	}
}
void entryDB::removeFieldValue(Entry& entry, size_t fieldIndex, const recordField& value) {
	// This function only concerns vectors
	// Need to first check if the value is a valid one, then we check if it exists in the field. 
	if (!std::holds_alternative<vecPTR>(EntryMeta[fieldIndex].ptr)) {
		throw std::runtime_error("Remove value only works with vector fields: " + EntryMeta[fieldIndex].info.name);
	}

	if (!std::holds_alternative<std::vector<int>>(value)) {
		throw std::runtime_error("Remove field value expects a Vector of integers");
	}

	try {
		validateField(value, fieldIndex);
	}
	catch (const std::exception& e) {
		std::cerr << "Error validating field \"" << EntryMeta[fieldIndex].info.name
			<< "\" for entry ID " << entry.id << ": " << e.what() << "\n";
		throw; // Rethrow to allow caller to handle as well
	}

	auto& field = entry.*std::get<vecPTR>(EntryMeta[fieldIndex].ptr);
	for (int removeVal : std::get<std::vector<int>>(value)) {
		// Process each value in the vector
		std::erase(field, removeVal);
		std::clog << "Removed value " << removeVal << " from field \"" << EntryMeta[fieldIndex].info.name
			<< "\" for entry ID " << entry.id << ".\n";
	}

	try {
		validateField(field, fieldIndex);
	}
	catch (const std::exception& e) {
		std::cerr << "Error validating field \"" << EntryMeta[fieldIndex].info.name
			<< "\" for entry ID " << entry.id << ": " << e.what() << "\n";
		throw; // Rethrow to allow caller to handle as well
	}
}
void entryDB::clearField(Entry& entry, size_t fieldIndex) {
	// Type Dynamic function
	if (EntryMeta[fieldIndex].info.presence == fieldPresence::REQUIRED) {
		throw std::runtime_error("Cannot clear required field: " + EntryMeta[fieldIndex].info.name);
	}

	switch (EntryMeta[fieldIndex].info.type) {
	case schemaField::dataType::U8: {
		entry.*std::get<intPTR>(EntryMeta[fieldIndex].ptr) = 0;
		break;
	}
	case schemaField::dataType::U16: {
		entry.*std::get<intPTR>(EntryMeta[fieldIndex].ptr) = 0;
		break;
	}
	case schemaField::dataType::U32: {
		entry.*std::get<intPTR>(EntryMeta[fieldIndex].ptr) = 0;
		break;
	}
	case schemaField::dataType::STR: {
		entry.*std::get<strPTR>(EntryMeta[fieldIndex].ptr) = "";
		break;
	}
	case schemaField::dataType::VECU8:
	case schemaField::dataType::VECU16: {
		entry.*std::get<vecPTR>(EntryMeta[fieldIndex].ptr) = {};
		break;
	}
	default:
		throw std::runtime_error("Unsupported data type for clear field operation: " + EntryMeta[fieldIndex].info.name);
	}
}
std::string entryDB::normalizeString(const std::string& string) {
	std::string buffer = string;
	std::transform(buffer.begin(), buffer.end(), buffer.begin(), [](unsigned char c) {
		return std::tolower(c);
	});
	return buffer;
}
std::vector<size_t> entryDB::idToIndex(std::vector<EntryId> ids) {
	std::vector<size_t> result;
	for (EntryId id : ids) {
		if (idMap.find(id) != idMap.end()) {
			result.push_back(idMap.find(id)->second);
		}
	}
	return result;
}



//====================================
//			Query Ops
//====================================
std::vector<EntryId> entryDB::getFilteredIDS(const FilterRequest& req) const {
	filterProcessor processor;

	return processor.query(req, *this);
}
// Non Functional
std::vector<EntryId> entryDB::getSortedIDS() {



	return {};
}



//====================================
//			Index Construction
//====================================
void entryDB::constructIdMap(std::unordered_map<EntryId, size_t>& idMap) const {
	idMap.clear();
	for (size_t i = 0; i < database.size(); i++) {
		idMap.emplace(database[i].id, i);
	}
}
void entryDB::constructTableIndexes(IndexStructure& tableIndexes) const {
	tableIndexes.clear();

	for (const EntryField& meta : EntryMeta) {
		if (meta.info.reference != ValueRef::LOOKUP_TABLE) {
			continue;
		}

		const std::string& tableTitle = meta.info.tableTitle;

		auto table = lookupTables.find(tableTitle);
		if (table == lookupTables.end()) {
			throw std::runtime_error("Missing lookup table: " + tableTitle);
		}

		auto& lookupIndex = tableIndexes[tableTitle];
		lookupIndex.reserve(table->second.size());

		for (LookupId lookupId = 0; lookupId < table->second.size(); lookupId++) {
			lookupIndex.emplace(lookupId, std::vector<EntryId>{});
		}

		for (const Entry& entry : database) {
			EntryId entryId = static_cast<EntryId>(entry.id);

			if (auto ptr = std::get_if<intPTR>(&meta.ptr)) {
				int value = entry.*(*ptr);

				if (value < 0 || static_cast<size_t>(value) >= table->second.size()) {
					throw std::runtime_error("Invalid lookup value in field: " + meta.info.name);
				}

				lookupIndex[static_cast<LookupId>(value)].push_back(entryId);
			}
			else if (auto ptr = std::get_if<vecPTR>(&meta.ptr)) {
				const std::vector<int>& values = entry.*(*ptr);

				for (int value : values) {
					if (value < 0 || static_cast<size_t>(value) >= table->second.size()) {
						throw std::runtime_error("Invalid lookup value in field: " + meta.info.name);
					}

					lookupIndex[static_cast<LookupId>(value)].push_back(entryId);
				}
			}
			else {
				throw std::runtime_error("Lookup table field is not int or vector<int>: " + meta.info.name);
			}
		}
	}
}
EntryId entryDB::addId() {
	if (idMap.find(nextId) != idMap.end())
	{
		throw std::runtime_error("Next Id is invalid!");
	}
	EntryId result = nextId;
	nextId++;
	return result;
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



//====================================
//			Validation
//====================================
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
				<< " not in EntryMeta.\n";
			throw std::runtime_error("Schema field not found in EntryMeta: " + field.name);
		}

		size_t metaIndex = found->second;

		if (EntryMeta[metaIndex].info.type != field.type) {
			std::cerr << "Schema field: \"" << field.name
				<< "\" at index: " << i
				<< " has incorrect data type.\n";
			throw std::runtime_error("Schema field has incorrect data type: " + field.name);
		}

		fieldIndices.push_back(metaIndex);
	}

	for (size_t i = 0; i < EntryMeta.size(); i++) {
		const FieldInfo& meta = EntryMeta[i].info;

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
void entryDB::validateTable(const std::string& title) {
	for (size_t i = 0; i < EntryMeta.size(); i++) {
		if (EntryMeta[i].info.tableTitle == title) {
			if (EntryMeta[i].info.reference == ValueRef::LOOKUP_TABLE) {
				return;
			} 
			std::cerr << "Field \"" << EntryMeta[i].info.name << "\" does not need a reference table but one is provided. \n";
			return;
		}
	}
	throw std::runtime_error("No field matches the table: " + title);
}
void entryDB::validateField(const recordField& value, const size_t fieldIndex) {
	const FieldInfo& meta = EntryMeta[fieldIndex].info;
	switch (meta.type) {
		case schemaField::dataType::U8: {
			int intValue = std::get<int>(value);
			try {
				validateScalar(intValue, meta);
			}
			catch (const std::exception& e) {
				throw std::runtime_error("Validation error for field \"" + meta.name + "\": " + e.what());
			}
			break;
		}
		case schemaField::dataType::U16: {
			int intValue = std::get<int>(value);
			try {
				validateScalar(intValue, meta);
			}
			catch (const std::exception& e) {
				throw std::runtime_error("Validation error for field \"" + meta.name + "\": " + e.what());
			}
			break;
		}
		case schemaField::dataType::U32: {
			int intValue = std::get<int>(value);
			try {
				validateScalar(intValue, meta);
			}
			catch (const std::exception& e) {
				throw std::runtime_error("Validation error for field \"" + meta.name + "\": " + e.what());
			}
			break;
		}
		case schemaField::dataType::STR: {
			// Ensure that value is a string
			if (!std::holds_alternative<std::string>(value)) {
				throw std::runtime_error("Expected string value for field: " + meta.name);
			}
			break;
		}
		case schemaField::dataType::VECU8: {
			std::vector<int> vecValue = std::get<std::vector<int>>(value);
			for (int val : vecValue) {
				try {
					validateScalar(val, meta);
				}
				catch (const std::exception& e) {
					throw std::runtime_error("Validation error for field \"" + meta.name + "\": " + e.what());
				}
			}
			break;
		}
		case schemaField::dataType::VECU16: {
			std::vector<int> vecValue = std::get<std::vector<int>>(value);
			for (int val : vecValue) {
				try {
					validateScalar(val, meta);
				}
				catch (const std::exception& e) {
					throw std::runtime_error("Validation error for field \"" + meta.name + "\": " + e.what());
				}
			}
			break;
		}
		default:
			throw std::runtime_error("Unknown data type for field: " + EntryMeta[fieldIndex].info.name);
	}
}
void entryDB::validateScalar(const int value, const FieldInfo& meta) {
	switch (meta.reference) {
		case ValueRef::LOOKUP_TABLE: {
			if (value < 0) {
				throw std::runtime_error("Negative value for table reference field: " + meta.name);
			}
			size_t tableSize = getTableSize(meta.tableTitle);
			if (value >= tableSize) {
				throw std::runtime_error("Value for field \"" + meta.name + "\" out of bounds of its reference table.");
			}
			break;
		}
		case ValueRef::BOUNDS: {
			if (!meta.bounds.has_value()) {
				throw std::runtime_error("Missing bounds for range field: " + meta.name);
			}
			if (value < meta.bounds->lower || value > meta.bounds->upper) {
				throw std::runtime_error("Value for field \"" + meta.name + "\" out of specified range.");
			}
			break;
		}
		case ValueRef::NONE: {
			break;
		}
		default:
			throw std::runtime_error("Unknown value reference type for field: " + meta.name);
	}
}



//====================================
//			Utility
//====================================
// Kinda want to re-write cause this uses ai code
recordField entryDB::getField(size_t id, size_t fieldIndex) const {
	size_t index = idMap.at(id);
	return getFromEntry(database[index], fieldIndex);
}
recordField entryDB::getFromEntry(const Entry& entry, const size_t fieldIndex) const {
	recordField result;

	const memPTR& ptr = EntryMeta[fieldIndex].ptr;

	std::visit([&](auto memberPtr) {
		result = entry.*memberPtr;
		}, ptr);

	return result;
}
size_t entryDB::getTableSize(const std::string& title) {
	return lookupTables.at(title).size();
}
std::unordered_map<std::string, size_t> buildFieldNames() {
	std::unordered_map<std::string, size_t> result;

	for (size_t i = 0; i < EntryMeta.size(); i++) {
		result.emplace(EntryMeta[i].info.name, i);
	}

	return result;
}



//====================================
//			Serialization
//====================================
// May want to re-write
// Check This later 
// 5/19/26 16:48
std::vector<uint8_t> entrySerializer::payload(entryDB& db, const std::vector<EntryId>& ids) {
	std::vector<uint8_t> result;
	
	std::vector<uint8_t> buffer = Section(db.getSection(fileType::HEADER_DATABASE, 0));

	result.insert(result.end(), buffer.begin(), buffer.end());

	buffer.clear();
	buffer = Schema(db.getSchema(fileType::HEADER_DATABASE, 0));

	result.insert(result.end(), buffer.begin(), buffer.end());
	buffer.clear();

	std::vector<Entry> entries = db.getEntry(ids);

	for (const Entry& entry : entries) {
		buffer = serializeEntry(entry);
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
Entry entrySerializer::readEntry(ByteReader& reader, std::vector<size_t>& fieldIndices) {
	Entry result;

	for (size_t i = 0; i < fieldIndices.size(); i++) {
		switch (EntryMeta[fieldIndices[i]].info.type) {
		case schemaField::dataType::U8:
			result.*std::get<intPTR>(EntryMeta[fieldIndices[i]].ptr) = reader.U8();
			break;
		case schemaField::dataType::U16:
			result.*std::get<intPTR>(EntryMeta[fieldIndices[i]].ptr) = reader.U16();
			break;
		case schemaField::dataType::U32:
			result.*std::get<intPTR>(EntryMeta[fieldIndices[i]].ptr) = reader.U32();
			break;
		case schemaField::dataType::STR:
			result.*std::get<strPTR>(EntryMeta[fieldIndices[i]].ptr) = reader.STR();
			break;
		case schemaField::dataType::VECU8:
			result.*std::get<vecPTR>(EntryMeta[fieldIndices[i]].ptr) = reader.VECU8();
			break;
		case schemaField::dataType::VECU16:
			result.*std::get<vecPTR>(EntryMeta[fieldIndices[i]].ptr) = reader.VECU16();
			break;
		default:
			throw std::runtime_error("Unknown data type in schema for field: " + EntryMeta[fieldIndices[i]].info.name);

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
std::vector<uint8_t> entrySerializer::serializeEntry(const Entry& entry) {
	ByteWriter writer;

	for (size_t i = 0; i < EntryMeta.size(); i++) {
		switch (EntryMeta[i].info.type) {
		case schemaField::dataType::U8:
			writer.U8(entry.*std::get<intPTR>(EntryMeta[i].ptr));
			break;
		case schemaField::dataType::U16:
			writer.U16(entry.*std::get<intPTR>(EntryMeta[i].ptr));
			break;
		case schemaField::dataType::U32:
			writer.U32(entry.*std::get<intPTR>(EntryMeta[i].ptr));
			break;
		case schemaField::dataType::STR:
			writer.STR(entry.*std::get<strPTR>(EntryMeta[i].ptr));
			break;
		case schemaField::dataType::VECU8:
			writer.VECU8(entry.*std::get<vecPTR>(EntryMeta[i].ptr));
			break;
		case schemaField::dataType::VECU16:
			writer.VECU16(entry.*std::get<vecPTR>(EntryMeta[i].ptr));
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
EntryUpdate entrySerializer::translateUpdate(const Update& update) {
	EntryUpdate result;
	result.id = update.id;
	result.op = update.operation;
	result.fieldIndex = update.fieldIndex;

	switch (update.type) {
		case dataType::U8:
			result.value = static_cast<int>(std::get<uint8_t>(update.value));
			break;
		case dataType::U16:
			result.value = static_cast<int>(std::get<uint16_t>(update.value));
			break;
		case dataType::U32: {
			uint32_t raw = std::get<uint32_t>(update.value);
			if (raw > static_cast<uint32_t>(std::numeric_limits<int>::max())) {
				throw std::runtime_error("Update value exceeds entry integer range.");
			}
			result.value = static_cast<int>(raw);
			break;
		}
		case dataType::STR:
			result.value = std::get<std::string>(update.value);
			break;
		case dataType::VECU8:
		case dataType::VECU16:
			result.value = std::get<std::vector<int>>(update.value);
			break;
		default:
			throw std::runtime_error("Unknown update value data type.");
	}

	return result;
}



//====================================
//			Routing
//====================================
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
			
			std::vector<EntryId> reqIds = reader.VECU32(); // Convert into a usable vector of ids or single id

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
			ByteReader reader{ message.payload };
			std::vector<EntryUpdate> updates;

			try {
				size_t updateCount = reader.U32();
				updates.reserve(updateCount);

				for (size_t i = 0; i < updateCount; i++) {
					Update update = codec.decodeUpdate(reader);
					updates.push_back(serializer.translateUpdate(update));
				}

				context.entries.updateEntry(updates);
			}
			catch (const std::exception& e) {
				std::cerr << "Error updating entries: " << e.what() << "\n";
				std::string errorMsg = "Error updating entries: " + std::string(e.what());
				return { codec.construct(COM_TYPE::RESP, DOMAIN_TYPE::ENTRY, ACTION_TYPE::ACT_UPDATE, message.id, static_cast<uint32_t>(errorMsg.size()), std::vector<uint8_t>(errorMsg.begin(), errorMsg.end())), false };
			}

			return { codec.construct(COM_TYPE::RESP, DOMAIN_TYPE::ENTRY, ACTION_TYPE::ACT_UPDATE, message.id, 7, {'S', 'U', 'C', 'C', 'E', 'S', 'S'}), false };
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
					Entry newEntry = serializer.readEntry(reader, fieldIndices);
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
		case ACTION_TYPE::ACT_DELETE: {
			// Handle delete action for entry domain
			EntryId deleteId = reader.U32();

			try {
				context.entries.deleteEntry(deleteId);
			}
			catch (const std::exception& e) {
				std::cerr << "Error deleting entry: " << e.what() << "\n";
				std::string errorMsg = "Error deleting entry: " + std::string(e.what());
				return { codec.construct(COM_TYPE::RESP, DOMAIN_TYPE::ENTRY, ACTION_TYPE::ACT_DELETE, message.id, static_cast<uint32_t>(errorMsg.size()), std::vector<uint8_t>(errorMsg.begin(), errorMsg.end())), false };
			}

			return { codec.construct(COM_TYPE::RESP, DOMAIN_TYPE::ENTRY, ACTION_TYPE::ACT_DELETE, message.id, 7, {'S', 'U', 'C', 'C', 'E', 'S', 'S'}) , false };
			break;
		}
		case ACTION_TYPE::ACT_SEARCH: {
			std::vector<EntryId> entries;
			try {
				entries = context.entries.searchEntryTitles(reader.STR());
			}
			catch (const std::exception& e) {
				std::cerr << "Error searching entries: " << e.what() << "\n";
				std::string errorMsg = "Error searching entries: " + std::string(e.what());
				return { codec.construct(COM_TYPE::RESP, DOMAIN_TYPE::ENTRY, ACTION_TYPE::ACT_SEARCH, message.id, static_cast<uint32_t>(errorMsg.size()), std::vector<uint8_t>(errorMsg.begin(), errorMsg.end())), false };
			}
			ByteWriter w;
			w.VECU32(entries);
			std::vector<uint8_t> resp = w.buffer;

			return { codec.construct(COM_TYPE::RESP, DOMAIN_TYPE::ENTRY, ACTION_TYPE::ACT_SEARCH, message.id, static_cast<uint32_t>(resp.size()), resp), false };
			break;
		}
		case ACTION_TYPE::ACT_FILTER: {
			filterProcessor processor;
			std::vector<EntryId> entries;
			try {
				entries = context.entries.getFilteredIDS(processor.decode(reader));
			} catch (const std::exception& e) {
				std::cerr << "Error filtering entries: " << e.what() << "\n";
				std::string errorMsg = "Error filtering entries: " + std::string(e.what());
				return {codec.construct(COM_TYPE::RESP, DOMAIN_TYPE::ENTRY, ACTION_TYPE::ACT_FILTER, message.id, static_cast<uint32_t>(errorMsg.size()),std::vector<uint8_t>(errorMsg.begin(), errorMsg.end())),false};
			}
			ByteWriter w;
			w.VECU32(entries);
			std::vector<uint8_t> resp = w.buffer;
			return {codec.construct(COM_TYPE::RESP, DOMAIN_TYPE::ENTRY, ACTION_TYPE::ACT_FILTER, message.id, static_cast<uint32_t>(resp.size()), resp), false};
			break;
		}
		default:
			std::cerr << "Unknown action type: " << static_cast<int>(message.action) << "\n";
			break;
	}
	return result;
}
