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

class groupDB {
	public:
	    using IdType = GroupId;

		void load(const std::string& filename);
		void save(const std::string& filename);

		// Read Handlers
		void section(const sectionHeader& header);
		void schema(const sectionHeader& section, const schemaData& schema);
		void record(const sectionHeader& section, const recordData& record, const schemaData& schema);
		void table(const sectionHeader& section, const std::string& table);

		// Write Handlers
	    fileHeader getHeader(fileType type) const;
	    sectionHeader getSection(fileType type, size_t index) const;
	    schemaData getSchema(fileType type, size_t index) const;
	    recordData getRecord(fileType type, size_t schemaIndex, size_t recordIndex) const;
	    GroupId getId(size_t index) const;
	    size_t getFieldIndex(const std::string& fieldName) const;

		// Mutate Operations
	    void addGroup(const Group& group);
	    void updateGroup(const std::vector<GroupUpdate>& updates);
	    std::vector<Group> getGroups(std::vector<GroupId> ids) const;
	    void deleteGroup(const GroupId id);
	    std::vector<GroupId> searchGroupTitles(const std::string& term) const;

		// Query Operations
	    std::vector<GroupId> getFilteredIds(const FilterRequest& request) const;
	    std::vector<GroupId> getSortedIds() const;

		// Indexing


		// Validation


		// Utilities

	private:
	    std::unordered_map<GroupId, size_t> idMap;
	    GroupId nextId = 1;

		std::vector<Group> groups;
	    std::vector<size_t> fieldMap;
};

} // namespace anime::storage
