#pragma once

#include <unordered_map>
#include <vector>
#include <filesystem>
#include "domain/entry.hpp"
#include "io/fileHandling.hpp"
#include "filtering/filterProcessor.hpp"

namespace anime::storage {

using anime::domain::USER_ACCOUNT_ROLE;
using anime::domain::Entry;
using anime::domain::EntryId;
using anime::domain::GroupId;
using anime::domain::IndexStructure;
using anime::domain::entryMetadata;
using anime::domain::entryFieldNames;
using anime::domain::EntryUpdate;
using anime::schema::FieldInfo;
using anime::filtering::FilterRequest;
using anime::filtering::filterProcessor;

template <typename T> struct is_vector : std::false_type {};
template <typename T, typename Allocator> struct is_vector<std::vector<T, Allocator>> : std::true_type {};

class groupDB;

class entryDB {
	public:
		using IdType = EntryId;

		// File load/save entry points
		void loadEntries(const std::filesystem::path& file);
		void loadLookupTables(const std::filesystem::path& file);
		void saveEntries(const std::filesystem::path& file);

		// Debug
		inline void printAll() {
			for (const auto& entry : database) {
				std::cout << '{';
				for (const auto& field : entryMetadata) {
					std::visit(
				    [&](const auto& memberPtr) {
					    const auto& value = entry.*memberPtr;
					    using ValueType = std::remove_cvref_t<decltype(value)>;
						if constexpr (std::same_as<ValueType, std::string>) {
							std::cout << value;
						} else if constexpr (storage::helpers::UnsignedVector<ValueType>) {
							std::cout << '{';
							for (const auto& val : value) {
								std::cout << std::to_string(static_cast<std::uint64_t>(val)) << ',';
							}
							std::cout << '}';
						} else if constexpr (storage::helpers::UnsignedInteger<ValueType>) {
							std::cout << std::to_string(static_cast<std::uint64_t>(value));
						} else if constexpr (std::same_as<ValueType, bool>) {
							if (value) {
								std::cout << "True";
							} else
								std::cout << "False";
						} else
							throw std::runtime_error("Cant handle data type");
				    },
				    field.ptr); 
					std::cout << ',';
				}
				std::cout << '}' << "\n";
			}
		}

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
		void addMembership(GroupId group, EntryId entry);

		// Query operations
		std::vector<EntryId> getFilteredIDS(const FilterRequest& filters) const;
		std::vector<EntryId> getSortedIDS();

		// Index construction
		void constructIdMap(std::unordered_map<EntryId, size_t>& idMap) const;
		void constructTableIndexes(IndexStructure& tableIndexes) const;
		EntryId addId();
		bool rankInsert(int rank);

		// Validation
		void validateTable(const std::string& title);
		bool validateIds();
		bool validateRanks();
		bool validateTitles();
		bool validateRanges();
		bool validateReferences();
		bool validateExternal(const groupDB& groups);

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
		std::unordered_map<GroupId, std::vector<EntryId>> membershipMap;

		// Acceleration Structures
		IndexStructure TableIndexes;

};

} // namespace anime::storage
