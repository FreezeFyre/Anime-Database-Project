// Anime List database implementation.
// Created by Luke Raykovitz on 10/24/25.

#include <algorithm>
#include <cctype>
#include <domain/entry.hpp>
#include <domain/types.hpp>
#include <exception>
#include <filtering/filterProcessor.hpp>
#include <io/file_types.hpp>
#include <io/fileHandling.hpp>
#include <iostream>
#include <schema/field.hpp>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>
#include "databases/entry_database.hpp"
#include "database_ops.hpp"
#include "domain/entry_schema.hpp"

namespace anime::storage {

using namespace anime::domain;
using namespace anime::filtering;
using namespace anime::schema;
using anime::storage::helpers::validateField;




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
	EntryId id = std::get<EntryId>(record[0]);
	EntryId expectedId = database.size();
	if (id != expectedId) {
		throw std::runtime_error("Non-sequential or corrupt ID in file. \n");
	}

	Entry entry{};
	for (size_t i = 0; i < fieldMap.size(); i++) {
		const auto& fieldInfo = entryMetadata[fieldMap[i]].info;

		if (std::holds_alternative<std::monostate>(record[i])) {
			if (fieldInfo.presence == FIELD_PRESENCE::REQUIRED) {
				throw std::runtime_error("Missing required data for field: " + fieldInfo.name + " at index: " + std::to_string(i) + "\n");
			} else { continue; }
		}
		std::visit([&](auto member, const auto& value) {
			using MemberType = std::remove_cvref_t<decltype(entry.*member)>;
			using ValueType = std::remove_cvref_t<decltype(value)>;
			
			if constexpr (std::is_same_v<MemberType, ValueType>) {
				entry.*member = value;
			} else if constexpr (std::is_integral_v<MemberType> && std::is_integral_v<ValueType>) { // Handles non-exact typing(needed for current dynamic width parsing)
				if constexpr (std::is_same_v<MemberType, bool>) {
					if (value == 0) {
						entry.*member = false;
					} else if (value == 1) {
						entry.*member = true;
					} else {
						throw std::runtime_error("Expected 1 or 0 for bool field.\n");
					}
				} else if (std::in_range<MemberType>(value)) {
					entry.*member = static_cast<MemberType>(value);
				}
			}
			
			
			else {
				throw std::runtime_error("Type mismatch for field '" + fieldInfo.name + "': expected " + typeid(MemberType).name() + ", got " + typeid(ValueType).name());
			}
		}, entryMetadata[fieldMap[i]].ptr, record[i]);
	}


	size_t index = database.size();
	database.emplace_back(std::move(entry));
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
	result.fieldCount = entryMetadata.size();
	result.hasSchema = true;

	return result;
}
sectionHeader entryDB::getSection() {
	sectionHeader result;

	result.type = sectionType::SECTION_DATABASE_RECORDS; // Placeholder
	result.title = "ANIME_DATA";
	result.recordCount = database.size();
	result.fieldCount = entryMetadata.size();
	result.hasSchema = true;

	return result;
}
schemaData entryDB::getSchema(fileType type, size_t sectionIndex) {
	schemaData result;
	schemaField buffer;

	for (size_t i = 0; i < entryMetadata.size(); i++) {
		buffer.name = entryMetadata[i].info.name;
		buffer.type = entryMetadata[i].info.type;
		buffer.mode = entryMetadata[i].info.mode;

		result.push_back(buffer);
	}

	return result;
}
schemaData entryDB::getSchema() {
	schemaData result;
	schemaField buffer;

	for (size_t i = 0; i < entryMetadata.size(); i++) {
		buffer.name = entryMetadata[i].info.name;
		buffer.type = entryMetadata[i].info.type;
		buffer.mode = entryMetadata[i].info.mode;

		result.push_back(buffer);
	}

	return result;
}
recordData entryDB::getRecord(fileType type, size_t sectionIndex, size_t recordIndex) {
	recordData result;

	for (size_t i = 0; i < entryMetadata.size(); i++) {
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
void entryDB::addEntry(const Entry& entry, const USER_ACCOUNT_ROLE& role) {
	if (role == USER_ACCOUNT_ROLE::VIEWER) {
		throw std::runtime_error("User does not meet account permission level to add entries. \n");
		return;
	}

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

	for (size_t i = 2; i < entryMetadata.size(); i++) {
		validateField(entryMetadata[i].info, getFromEntry(result, i));
	}
	size_t index = database.size();
	database.push_back(result);
	idMap.emplace(static_cast<EntryId>(result.id), index);
}
void entryDB::updateEntry(const std::vector<EntryUpdate>& updates, const USER_ACCOUNT_ROLE& role) {
	if (role == USER_ACCOUNT_ROLE::VIEWER) {
		throw std::runtime_error("User does not meet account permission level to update entries. \n");
		return;
	}

	for (const EntryUpdate& update : updates) {
		if (update.fieldIndex >= entryMetadata.size()) {
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
			case UPDATE_OPERATION::SET_FIELD: {
				try {
					setField(database[index], update.fieldIndex, update.value);
				}
				catch (const std::exception& e) {
					std::cerr << "Error Setting Entry" << update.id << "Field: " << update.fieldIndex << ": " << e.what() << ".\n";
					throw; // Rethrow to allow caller to handle as well
				}
				break;
			}
			case UPDATE_OPERATION::CLEAR_FIELD: {
				try {
					clearField(database[index], update.fieldIndex);
				}
				catch (const std::exception& e) {
					std::cerr << "Error Clearing Entry" << update.id << "Field: " << update.fieldIndex << ": " << e.what() << ".\n";
					throw; // Rethrow to allow caller to handle as well
				}
				break;
			}
			case UPDATE_OPERATION::ADD_VALUE: {
				if (entryMetadata[update.fieldIndex].info.type != schemaField::dataType::VECU8 and entryMetadata[update.fieldIndex].info.type != schemaField::dataType::VECU16) {
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
			case UPDATE_OPERATION::REMOVE_VALUE: {
				if (entryMetadata[update.fieldIndex].info.type != schemaField::dataType::VECU8 and entryMetadata[update.fieldIndex].info.type != schemaField::dataType::VECU16) {
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
std::vector<Entry> entryDB::getEntries(std::vector<EntryId> ids) {
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
void entryDB::deleteEntry(const EntryId id, const USER_ACCOUNT_ROLE& role) {
	if (role == USER_ACCOUNT_ROLE::VIEWER) {
		throw std::runtime_error("User does not meet account permission level to delete entries. \n");
		return;
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
		// Should eventually make this search all string fields in entryMetadata that have a qualifer like "SEARCHABLE"
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
		validateField(entryMetadata[fieldIndex].info, value);
	}
	catch (const std::exception& e) {
		std::cerr << "Error validating field \"" << entryMetadata[fieldIndex].info.name
			<< "\" for entry ID " << entry.id << ": " << e.what() << "\n";
		throw; // Rethrow to allow caller to handle as well
	}
	if (std::holds_alternative<std::monostate>(value)) {
		throw std::runtime_error("Cant have emtpy data for set field operation: " + entryMetadata[fieldIndex].info.name);
	}
	std::visit([&](auto member, const auto& data) {
		using MemberType = std::remove_cvref_t<decltype(entry.*member)>;
		using DataType = std::remove_cvref_t<decltype(data)>;

		if constexpr (std::is_same_v<MemberType, DataType>) {
				entry.*member = data;
		} else if constexpr (std::is_integral_v<MemberType> && std::is_integral_v<DataType>) { // Handles non-exact typing(needed for current dynamic width parsing)
			if constexpr (std::is_same_v<MemberType, bool>) {
				if (data == 0) {
					entry.*member = false;
				} else if (data == 1) {
					entry.*member = true;
				} else {
					throw std::runtime_error("Expected 1 or 0 for bool field.\n");
				}
			} else if (std::in_range<MemberType>(data)) {
				entry.*member = static_cast<MemberType>(data);
			}
		}

	}, entryMetadata[fieldIndex].ptr, value);
}
void entryDB::addFieldValue(Entry& entry, size_t fieldIndex, const recordField& value) {
	// Should probably re-write using helper functions and making better use of validateField, should probably also improve validate field function
	std::visit([&](const auto& memberPtr, const auto& additive){
		using MemberType = std::remove_cvref_t<decltype(memberPtr)>;
		using ValueType = std::remove_cvref_t<decltype(additive)>;
		if constexpr (!is_vector<MemberType>::value>) {
			throw std::runtime_error(" Add field value is a vector only function. ");
		} else {
			using MemberElementType = typename MemberType::value_type;
			if (!std::unsigned_integral(MemberElementType)) {
				throw std::runtime_error(" Expects only unsigned integers in members. ");
			}
			if constexpr ( is_vector<ValueType>::value) {
				using ValueElementType = typename ValueType::value_type;
				if (!std::unsigned_integral(ValueElementType)) {
					throw std::runtime_error(" Expects only unsigned integers in value input. ");
				}

				if constexpr (std::in_range<MemberElementType>(additive[0])) {
					for (const auto& fieldVal : additive) {
						entry.*memberPtr.push_back(fieldVal);
						std::clog << "Added value " << std::to_string(static_cast<std::uint64_t>(fieldVal)) << " to field \"" << entryMetadata[fieldIndex].info.name << "\" for entry ID " << std::to_string(entry.id) << ".\n";
					}
				}

			} else if (std::unsigned_integral(ValueType) && std::in_range<MemberElementType>(additive)) {
				entry.*memberPtr.push_back(additive);
				std::clog << "Added value " << std::to_string(static_cast<std::uint64_t>(additive)) << " to field \"" << entryMetadata[fieldIndex].info.name << "\" for entry ID " << std::to_string(entry.id) << ".\n";
			} else {
				throw std::runtime_error(" Invalid type to add to the field. ");
			}
		}
	}, entryMetadata[fieldIndex].ptr, value);
}
void entryDB::removeFieldValue(Entry& entry, size_t fieldIndex, const recordField& value) {
	// This function only concerns vectors
	// Need to first check if the value is a valid one, then we check if it exists in the field. 
	if (!std::holds_alternative<vecPTR>(entryMetadata[fieldIndex].ptr)) {
		throw std::runtime_error("Remove value only works with vector fields: " + entryMetadata[fieldIndex].info.name);
	}

	if (!std::holds_alternative<std::vector<int>>(value)) {
		throw std::runtime_error("Remove field value expects a Vector of integers");
	}

	try {
		validateField(entryMetadata[fieldIndex].info, value);
	}
	catch (const std::exception& e) {
		std::cerr << "Error validating field \"" << entryMetadata[fieldIndex].info.name
			<< "\" for entry ID " << entry.id << ": " << e.what() << "\n";
		throw; // Rethrow to allow caller to handle as well
	}

	auto& field = entry.*std::get<vecPTR>(entryMetadata[fieldIndex].ptr);
	for (int removeVal : std::get<std::vector<int>>(value)) {
		// Process each value in the vector
		std::erase(field, removeVal);
		std::clog << "Removed value " << removeVal << " from field \"" << entryMetadata[fieldIndex].info.name
			<< "\" for entry ID " << entry.id << ".\n";
	}

	try {
		validateField(field, fieldIndex);
	}
	catch (const std::exception& e) {
		std::cerr << "Error validating field \"" << entryMetadata[fieldIndex].info.name
			<< "\" for entry ID " << entry.id << ": " << e.what() << "\n";
		throw; // Rethrow to allow caller to handle as well
	}
}
void entryDB::clearField(Entry& entry, size_t fieldIndex) {
	// Type Dynamic function
	if (entryMetadata[fieldIndex].info.presence == FIELD_PRESENCE::REQUIRED) {
		throw std::runtime_error("Cannot clear required field: " + entryMetadata[fieldIndex].info.name);
	}

	switch (entryMetadata[fieldIndex].info.type) {
	case schemaField::dataType::U8: {
		entry.*std::get<intPTR>(entryMetadata[fieldIndex].ptr) = 0;
		break;
	}
	case schemaField::dataType::U16: {
		entry.*std::get<intPTR>(entryMetadata[fieldIndex].ptr) = 0;
		break;
	}
	case schemaField::dataType::U32: {
		entry.*std::get<intPTR>(entryMetadata[fieldIndex].ptr) = 0;
		break;
	}
	case schemaField::dataType::STR: {
		entry.*std::get<strPTR>(entryMetadata[fieldIndex].ptr) = "";
		break;
	}
	case schemaField::dataType::VECU8:
	case schemaField::dataType::VECU16: {
		entry.*std::get<vecPTR>(entryMetadata[fieldIndex].ptr) = {};
		break;
	}
	default:
		throw std::runtime_error("Unsupported data type for clear field operation: " + entryMetadata[fieldIndex].info.name);
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

	for (const EntryField& meta : entryMetadata) {
		if (meta.info.reference != VALUE_REFERENCE_TYPE::LOOKUP_TABLE) {
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
	return anime::domain::validateEntrySchema(schema);
}
void entryDB::validateTable(const std::string& title) {
	for (size_t i = 0; i < entryMetadata.size(); i++) {
		if (entryMetadata[i].info.tableTitle == title) {
			if (entryMetadata[i].info.reference == VALUE_REFERENCE_TYPE::LOOKUP_TABLE) {
				return;
			} 
			std::cerr << "Field \"" << entryMetadata[i].info.name << "\" does not need a reference table but one is provided. \n";
			return;
		}
	}
	throw std::runtime_error("No field matches the table: " + title);
}
void entryDB::validateScalar(const int value, const FieldInfo& meta) {
	switch (meta.reference) {
		case VALUE_REFERENCE_TYPE::LOOKUP_TABLE: {
			if (value < 0) {
				throw std::runtime_error("Negative value for table reference field: " + meta.name);
			}
			size_t tableSize = getTableSize(meta.tableTitle);
			if (value >= tableSize) {
				throw std::runtime_error("Value for field \"" + meta.name + "\" out of bounds of its reference table.");
			}
			break;
		}
		case VALUE_REFERENCE_TYPE::BOUNDS: {
			if (!meta.bounds.has_value()) {
				throw std::runtime_error("Missing bounds for range field: " + meta.name);
			}
			if (value < meta.bounds->lower || value > meta.bounds->upper) {
				throw std::runtime_error("Value for field \"" + meta.name + "\" out of specified range.");
			}
			break;
		}
		case VALUE_REFERENCE_TYPE::NONE: {
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

	const memPTR& ptr = entryMetadata[fieldIndex].ptr;

	std::visit([&](auto memberPtr) {
		result = entry.*memberPtr;
		}, ptr);

	return result;
}
size_t entryDB::getTableSize(const std::string& title) {
	return lookupTables.at(title).size();
}


} // namespace anime::storage
