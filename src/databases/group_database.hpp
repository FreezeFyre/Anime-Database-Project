#pragma once

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <cctype>
#include <algorithm>
#include <limits>
#include <variant>
#include <unordered_map>
#include <unordered_set>
#include <array>
#include <typeinfo>
#include <optional>

#include "io/fileHandling.hpp"
#include "domain/types.hpp"
#include "schema/field.hpp"
#include "filtering/filterProcessor.hpp"
#include "net/protocol/data.hpp"


#include "domain/group.hpp"

namespace anime::storage {
using anime::domain::Group;
using anime::domain::GroupId;
using anime::domain::GroupUpdate;

using anime::filtering::FilterRequest;
using anime::filtering::filterProcessor;

class entryDB;

class groupDB {
	public:
	    using IdType = GroupId;

		void load(const std::filesystem::path& filename);
		void compile(const entryDB& entries);
		void save(const std::filesystem::path& filename);
		inline void printAll() {
			for (const auto& group : groups) {
				std::cout << '{';
				for (const auto& field : anime::domain::groupMetadata) {
					std::visit(
				    [&](const auto& memberPtr) {
					    const auto& value = group.*memberPtr;
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

		// Read Handlers
		void section(const sectionHeader& header);
		void schema(const sectionHeader& section, const schemaData& schema);
		void record(const sectionHeader& section, const recordData& record, const schemaData& schema);
		void table(const sectionHeader& section, const std::string& table);


		// Write Handlers
	    fileHeader getHeader(fileType type) const;
	    sectionHeader getSection(fileType type, size_t index) const;
		sectionHeader getSection() const;
	    schemaData getSchema(fileType type, size_t index) const;
		schemaData getSchema() const;
	    recordData getRecord(fileType type, size_t schemaIndex, size_t recordIndex) const;
		recordData getTableRecord(fileType type, size_t sectionIndex, size_t rowIndex);
	    GroupId getId(size_t index) const;
	    size_t getFieldIndex(const std::string& fieldName) const;

		// Mutate Operations
	    void addGroup(const Group& group);
	    void updateGroup(const std::vector<GroupUpdate>& updates);
	    std::vector<Group> getGroups(std::vector<GroupId> ids) const;
	    void deleteGroup(const GroupId id);
	    std::vector<GroupId> searchGroupTitles(const std::string& term) const;

		// Operation Utilities

		// Query Operations
	    std::vector<GroupId> getFilteredIds(const FilterRequest& request) const;
	    std::vector<GroupId> getSortedIds() const;
		inline bool contains(GroupId id) const {
			return idMap.contains(id);
		}

		// Indexing


		// Validation
		std::vector<size_t> validateSchema(const schemaData& schema);

		// Utilities
		recordField getField(size_t id, size_t fieldIndex) const;
		recordField getFromGroup(const Group& group, const size_t fieldIndex) const;

	private:
	    std::unordered_map<GroupId, size_t> idMap;
	    GroupId nextId = 1;

		std::vector<Group> groups;
	    std::vector<size_t> fieldMap;
};

} // namespace anime::storage
