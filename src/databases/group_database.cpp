#include "databases/group_database.hpp"
#include "../domain/group_schema.hpp"
#include "../domain/group_compiler.hpp"
#include "validation.hpp"
#include "database_ops.hpp"

#include <variant>
#include <type_traits>

namespace anime::storage {

using namespace anime::domain;
using namespace anime::filtering;
using namespace anime::schema;
using anime::domain::group::compileGroup;
using anime::storage::validation::validateField;
using namespace anime::storage::operators;

// Main I/O
void groupDB::load(const std::filesystem::path& filename) {
	fileProcessor parser;

	try {
		parser.readFile(filename, *this, fileType::HEADER_DATABASE);
	} catch (const std::exception& e) {
		std::cerr << "Error loading group database: " << e.what() << "\n";
		throw std::runtime_error(" Error loading database" + std::string(e.what()));
	}
}
void groupDB::save(const std::filesystem::path& filename) {
	if (!validation::validateUniqueness(groups, groupMetadata))
		throw std::runtime_error("Group uniqueness validation failed before save");
	fileProcessor parser;
	try {
		parser.saveFile(filename, *this, fileType::HEADER_DATABASE);
	} catch (const std::exception& e) {
		std::cerr << "Error saving group database: " << e.what() << "\n";
		throw std::runtime_error(" Error saving database" + std::string(e.what()));
	}

}

// Uniques
void groupDB::compile(const entryDB& entries) {
	for (auto& group : groups) {
		compileGroup(group, entries);
	}
}
void groupDB::compile(GroupId id, const entryDB& entries) {
	compileGroup(groups[idMap.at(id)], entries);
}
void groupDB::compile(const std::vector<GroupId>& ids, const entryDB& entries) {
	for (auto& id : ids) {
		compileGroup(groups[idMap.at(id)], entries);
	}
}
void groupDB::attachMembers(const std::unordered_map<GroupId, std::vector<EntryId>>& memberships) {
	for (const auto& [group, members] : memberships) {
		groups[idMap.at(group)].members = members;
	}
}

// Read Handlers
void groupDB::section(const sectionHeader & header) {
	if (header.type == sectionType::SECTION_DATABASE_RECORDS) {
		groups.reserve(header.recordCount);
	} else {
		throw std::runtime_error("Unsupported section header for groups. \n");
	}
}
void groupDB::schema(const sectionHeader& section, const schemaData& schema) {
	fieldMap.clear();
	fieldMap = validateSchema(schema);
}
void groupDB::record(const sectionHeader& section, const recordData& record, const schemaData& schema){
	using anime::storage::helpers::UnsignedInteger;

	Group group{};
	for (size_t i = 0; i < fieldMap.size(); i++) {
		const auto& fieldInfo = groupMetadata[fieldMap[i]].info;

		if (std::holds_alternative<std::monostate>(record[i])) {
			if (fieldInfo.presence == FIELD_PRESENCE::REQUIRED) {
				throw std::runtime_error("Missing required data for field: " + fieldInfo.name + " at index: " + std::to_string(i) + "\n");
			} else { continue; }
		}
		validateField(fieldInfo, record[i]);
		std::visit([&](auto member, const auto& value) {
			using MemberType = std::remove_cvref_t<decltype(group.*member)>;
			using ValueType = std::remove_cvref_t<decltype(value)>;
			
			if constexpr (std::is_same_v<MemberType, ValueType>) {
				group.*member = value;
			} else if constexpr (UnsignedInteger<MemberType> && UnsignedInteger<ValueType>) { // Handles non-exact typing(needed for current dynamic width parsing)
				group.*member = helpers::checkedConvert<MemberType>(value);
			} else {
				throw std::runtime_error("Type mismatch for field '" + fieldInfo.name + "': expected " + typeid(MemberType).name() + ", got " + typeid(ValueType).name());
			}
		}, groupMetadata[fieldMap[i]].ptr, record[i]);
	}

	validation::validateUniqueness(groups, group, groupMetadata);
	if (group.id >= nextId) {
		nextId = group.id + 1;
	}
	size_t index = groups.size();
	groups.emplace_back(std::move(group));
	idMap.emplace(static_cast<GroupId>(groups[index].id), index);

}
void groupDB::table(const sectionHeader&, const std::string&) {
	throw std::runtime_error("Lookup-table sections are not supported for groups.");
}

// Write Handlers
fileHeader groupDB::getHeader(fileType type) const {
	fileHeader result = {.type = type, .sectionCount = 1};
	return result;
}
sectionHeader groupDB::getSection(fileType type, size_t index) const {
	return sectionHeader{
		.type = sectionType::SECTION_DATABASE_RECORDS,
		.title = "GROUP_DATA",
		.recordCount = static_cast<std::uint32_t>(groups.size()),
		.fieldCount = groupMetadata.size(),
		.hasSchema = true
	};
}
sectionHeader groupDB::getSection() const {
	return sectionHeader{
		.type = sectionType::SECTION_DATABASE_RECORDS,
		.title = "GROUP_DATA",
		.recordCount = static_cast<std::uint32_t>(groups.size()),
		.fieldCount = groupMetadata.size(),
		.hasSchema = true
	};
}
schemaData groupDB::getSchema(fileType type, size_t index) const {
	schemaData result;
	schemaField buffer;

	for (size_t i = 0; i < groupMetadata.size(); i++) {
		buffer.name = groupMetadata[i].info.name;
		buffer.type = groupMetadata[i].info.type;
		buffer.mode = groupMetadata[i].info.mode;

		result.push_back(buffer);
	}

	return result;
}
schemaData groupDB::getSchema() const {
	schemaData result;
	schemaField buffer;

	for (size_t i = 0; i < groupMetadata.size(); i++) {
		buffer.name = groupMetadata[i].info.name;
		buffer.type = groupMetadata[i].info.type;
		buffer.mode = groupMetadata[i].info.mode;

		result.push_back(buffer);
	}

	return result;
}
recordData groupDB::getRecord(fileType type, size_t schemaIndex, size_t recordIndex) const {
	recordData result;

	for (size_t i = 0; i < groupMetadata.size(); i++) {
		recordField buffer;

		buffer = getField(groups[recordIndex].id, i);
		result.push_back(buffer);
	}

	return result;
}
recordData groupDB::getTableRecord(fileType type, size_t sectionIndex, size_t rowIndex) {
	return recordData();
}

// Indexing
void groupDB::constructIdMap(std::unordered_map<GroupId, size_t>& idMap) const {
	idMap.clear();
	for (size_t i = 0; i < groups.size(); i++) {
		idMap.emplace(groups[i].id, i);
	}
}

// Validation
std::vector<size_t> groupDB::validateSchema(const schemaData& schema) const {
	return anime::domain::validateGroupSchema(schema);
}

// Field Queries
recordField groupDB::getField(size_t id, size_t fieldIndex) const {
	size_t index = idMap.at(id);
	return getFromGroup(groups[index], fieldIndex);
}
recordField groupDB::getFromGroup(const Group& group, const size_t fieldIndex) const {
	const GroupMemberPtr& ptr = groupMetadata[fieldIndex].ptr;
	return std::visit([&](auto memberPtr) { return recordField{group.*memberPtr}; }, ptr);
}
std::vector<Group> groupDB::getGroups(const std::vector<GroupId>& ids) const {
	return ids |
		std::views::transform([&](const auto& id) -> const auto& { return groups[idMap.at(id)]; }) |
		std::ranges::to<std::vector<Group>>();
}


GroupId groupDB::addGroup(const Group& group) {
	// Need to compare to a name map eventually
	Group result = group;
	for (const auto& field : groupMetadata) {
		if (field.info.editPolicy == EDIT_POLICY::INTERNAL_ONLY)
			std::visit([&](auto ptr) { result.*ptr = Group{}.*ptr; }, field.ptr);
	}
	result.id = nextId;

	// Need to add rank checking

	for (size_t i = 2; i < groupMetadata.size(); i++) { // Id and rank are handled seperatly, should consider making a generic that uses metadata hints
		validateField(groupMetadata[i].info, getFromGroup(result, i));
	}

	validation::validateUniqueness(groups, result, groupMetadata);
	nextId++;
	groups.push_back(result);
	idMap.insert({result.id, groups.size()-1});
	return result.id;
}
std::vector<GroupId> groupDB::updateGroups(const std::vector<GroupUpdate>& updates) {

	std::unordered_set<GroupId> set;
	for (const auto& update : updates) {
		if (update.fieldIndex >= groupMetadata.size()) {throw std::runtime_error("Update field out of bounds");}
		validation::validateEditPolicy(groupMetadata[update.fieldIndex].info);
		if (update.fieldIndex == 1) { throw std::runtime_error("Rank updates are not supported yet."); }

		set.emplace(update.id);
		auto index = idMap.at(update.id);
		Group candidate = groups[index];
		switch (update.operation) {
			case UPDATE_OPERATION::SET_FIELD: {
				try {
					setField(candidate, update.fieldIndex, update.value);
				}
				catch (const std::exception& e) {
					std::cerr << "Error Setting Entry" << update.id << "Field: " << update.fieldIndex << ": " << e.what() << ".\n";
					throw; // Rethrow to allow caller to handle as well
				}
				break;
			}
			case UPDATE_OPERATION::CLEAR_FIELD: {
				try {
					clearField(candidate, update.fieldIndex);
				}
				catch (const std::exception& e) {
					std::cerr << "Error Clearing Entry" << update.id << "Field: " << update.fieldIndex << ": " << e.what() << ".\n";
					throw; // Rethrow to allow caller to handle as well
				}
				break;
			}
			case UPDATE_OPERATION::ADD_VALUE: {
				if (groupMetadata[update.fieldIndex].info.type != schemaField::dataType::VECU8 and groupMetadata[update.fieldIndex].info.type != schemaField::dataType::VECU16 and groupMetadata[update.fieldIndex].info.type != schemaField::dataType::VECU32) {
					throw std::runtime_error("Add value only works with vectors");
				}
				try {
					addFieldValue(candidate, update.fieldIndex, update.value);
				}
				catch (const std::exception& e) {
					std::cerr << "Error adding value to Entry" << update.id << "Field: " << e.what() << ".\n";
					throw; // Rethrow to allow caller to handle as well
				}
				break;
			}
			case UPDATE_OPERATION::REMOVE_VALUE: {
				if (groupMetadata[update.fieldIndex].info.type != schemaField::dataType::VECU8 and groupMetadata[update.fieldIndex].info.type != schemaField::dataType::VECU16 and groupMetadata[update.fieldIndex].info.type != schemaField::dataType::VECU32) {
					throw std::runtime_error("Remove value only works with vectors");
				}
				try {
					removeFieldValue(candidate, update.fieldIndex, update.value);
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
		validation::validateUniqueness(groups, candidate, groupMetadata, &groups[index]);
		groups[index] = std::move(candidate);
	}
	return {set.begin(), set.end()};
}
void groupDB::deleteGroup(const GroupId id) {
	auto index = idMap.at(id);

	// Rank Re-Ordering
	if (groups[index].rank != 0) {
		for (auto& group : groups) {
			if (group.rank > groups[index].rank) {
				group.rank--;
			}
		}
	}

	groups.erase(groups.begin() + index); // may want to use swap and popback or erase_if instead
	constructIdMap(idMap);
}

void groupDB::setField(Group& group, size_t fieldIndex, const recordField& value) {
	validation::validateEditPolicy(groupMetadata.at(fieldIndex).info);
	try {
		validateField(groupMetadata[fieldIndex].info, value);
	}
	catch (const std::exception& e) {
		std::cerr << "Error validating input for field \"" << groupMetadata[fieldIndex].info.name
			<< "\" for group ID " << group.id << ": " << e.what() << "\n";
		throw;
	}
	std::visit([&](auto memberPtr, const auto& input) {
		field_ops::setField(group.*memberPtr, input);
	}, groupMetadata[fieldIndex].ptr, value);
}
void groupDB::addFieldValue(Group& group, size_t fieldIndex, const recordField& value) {
	validation::validateEditPolicy(groupMetadata.at(fieldIndex).info);
	try {
		validateField(groupMetadata[fieldIndex].info, value);
	}
	catch (const std::exception& e) {
		std::cerr << "Error validating input for field \"" << groupMetadata[fieldIndex].info.name
			<< "\" for group ID " << group.id << ": " << e.what() << "\n";
		throw;
	}

	std::visit([&](const auto& memberPtr, const auto& input){
		using FieldType = std::remove_cvref_t<decltype(group.*memberPtr)>;
		if constexpr (helpers::is_vector_v<FieldType>) {
			vector_ops::addUniqueElement(group.*memberPtr, input);
		} else {
			throw std::runtime_error("Add requires a vector target.");
		}
	}, groupMetadata[fieldIndex].ptr, value);
}
void groupDB::removeFieldValue(Group& group, size_t fieldIndex, const recordField& value) {
	validation::validateEditPolicy(groupMetadata.at(fieldIndex).info);
	try {
		validateField(groupMetadata[fieldIndex].info, value);
	}
	catch (const std::exception& e) {
		std::cerr << "Error validating input for field \"" << groupMetadata[fieldIndex].info.name
			<< "\" for group ID " << group.id << ": " << e.what() << "\n";
		throw; // Rethrow to allow caller to handle as well
	}

	std::visit([&](const auto& memberPtr, const auto& input) {
		using FieldType = std::remove_cvref_t<decltype(group.*memberPtr)>;

		if constexpr (helpers::is_vector_v<FieldType>) {
			vector_ops::removeElement(group.*memberPtr, input);
		} else {
			throw std::runtime_error("Remove requires a vector target.");
		}
	}, groupMetadata[fieldIndex].ptr, value);
}
void groupDB::clearField(Group& group, size_t fieldIndex) {
	validation::validateEditPolicy(groupMetadata.at(fieldIndex).info);
	if (groupMetadata[fieldIndex].info.presence == FIELD_PRESENCE::REQUIRED) {
		throw std::runtime_error("Cannot clear required field: " + groupMetadata[fieldIndex].info.name);
	}

	std::visit([&](const auto& memberPtr) {
		field_ops::clearField(group.*memberPtr);
	}, groupMetadata[fieldIndex].ptr);
}

} // namespace anime::storage
