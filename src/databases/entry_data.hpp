#pragma once

#include <unordered_map>
#include <vector>
#include "domain/entry.hpp"
#include "io/fileHandling.hpp"
#include "filtering/filterProcessor.hpp"

namespace anime::storage {

using anime::domain::USER_ACCOUNT_ROLE;
using anime::domain::Entry;
using anime::domain::EntryId;
using anime::domain::IndexStructure;
using anime::domain::entryMetadata;
using anime::domain::entryFieldNames;
using anime::domain::EntryUpdate;
using anime::schema::FieldInfo;
using anime::filtering::FilterRequest;
using anime::filtering::filterProcessor;

class entryDB {
	public:
		using IdType = EntryId;

		// File load/save entry points
		void loadEntries(std::string file);
		void loadLookupTables(std::string file);
		void saveEntries(std::string file);


		// File read handlers
		void section(const sectionHeader& header);
		void schema(const sectionHeader& section, const schemaData& schema);
		void record(const sectionHeader& section, const recordData& record, const schemaData& schema);
		void table(const sectionHeader& section, const std::string& table);

		// File write sources
		fileHeader getHeader(fileType type);
		sectionHeader getSection(fileType type, size_t sectionIndex);
		sectionHeader getSection();
		schemaData getSchema(fileType type, size_t sectionIndex);
		schemaData getSchema();
		recordData getRecord(fileType type, size_t sectionIndex, size_t recordIndex);
		recordData getTableRecord(fileType type, size_t sectionIndex, size_t rowIndex);
		EntryId getId(size_t index) const {
			return static_cast<EntryId>(database[index].id);
		}
	    size_t getFieldIndex(const std::string& fieldName) const {
		    auto it = entryFieldNames.find(fieldName);
		    if (it != entryFieldNames.end()) {
			    return it->second;
		    }
		    throw std::runtime_error("Field name not found: " + fieldName);
	    }

		// Entry operations
		void addEntry(const Entry& entry, const USER_ACCOUNT_ROLE& role);
		void updateEntry(const std::vector<EntryUpdate>& updates, const USER_ACCOUNT_ROLE& role);
		std::vector<Entry> getEntries(std::vector<EntryId> ids);
		void deleteEntry(const EntryId id, const USER_ACCOUNT_ROLE& role);
		std::vector<EntryId> searchEntryTitles(const std::string& term);

		// Operation Utilities
		void setField(Entry& entry, size_t fieldIndex, const recordField& value);
		void addFieldValue(Entry& entry, size_t fieldIndex, const recordField& value);
		void removeFieldValue(Entry& entry, size_t fieldIndex, const recordField& value);
		void clearField(Entry& entry, size_t fieldIndex);
		std::string normalizeString(const std::string& string);
		std::vector<size_t> idToIndex(std::vector<EntryId> ids);

		// Query operations
		std::vector<EntryId> getFilteredIDS(const FilterRequest& filters) const;
		std::vector<EntryId> getSortedIDS();

		// Index construction
		void constructIdMap(std::unordered_map<EntryId, size_t>& idMap) const;
		void constructTableIndexes(IndexStructure& tableIndexes) const;
		EntryId addId();
		bool rankInsert(int rank);

		// Validation
		std::vector<size_t> validateSchema(const schemaData& schema);
		void validateTable(const std::string& title);
		void validateField(const recordField& value,const size_t fieldIndex);
		void validateScalar(const int value, const FieldInfo& meta);

		// Utility accessors
		const FieldInfo& getFieldInfo(size_t fieldIndex) const {
			return entryMetadata[fieldIndex].info;
		}
		size_t getSize() const { return database.size(); }
		recordField getField(size_t id, size_t fieldIndex) const;
		recordField getFromEntry(const Entry& entry, const size_t fieldIndex) const;
		size_t getFieldCount() const { return entryMetadata.size(); }
		size_t getTableSize(const std::string& title);

	private:
		std::unordered_map<EntryId, size_t> idMap; //Maps ids to their index in the vector allowing for database transforms
		EntryId nextId = 50;

		std::vector<Entry> database;
		std::vector<size_t> fieldMap;

		std::unordered_map<std::string, std::vector<std::string>> lookupTables;
		

		// Acceleration Structures
		IndexStructure TableIndexes;

};

} // namespace anime::storage
