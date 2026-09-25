#include "databases/group_database.hpp"
#include "../domain/group_schema.hpp"
#include "../domain/group_compiler.hpp"

#include <variant>
#include <type_traits>

namespace anime::storage {

using namespace anime::domain;
using namespace anime::filtering;
using namespace anime::schema;

void groupDB::load(const std::filesystem::path& filename) {
	fileProcessor parser;

	try {
		parser.readFile(filename, *this, fileType::HEADER_DATABASE);
	} catch (const std::exception& e) {
		std::cerr << "Error loading group database: " << e.what() << "\n";
	}
}
void groupDB::save(const std::filesystem::path& filename) {
	fileProcessor parser;
	try {
		parser.saveFile(filename, *this, fileType::HEADER_DATABASE);
	} catch (const std::exception& e) {
		std::cerr << "Error saving group database: " << e.what() << "\n";
	}

}

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
	// Check For ID, will probably change to check against a map of existing ids to ensure there arent duplicates
	GroupId id = std::get<GroupId>(record[0]);
	GroupId expectedId = groups.size();
	if (id != expectedId) {
		throw std::runtime_error("Non-sequential or corrupt ID in file. \n");
	}

	Group group{};
	for (size_t i = 0; i < fieldMap.size(); i++) {
		const auto& fieldInfo = groupMetadata[fieldMap[i]].info;

		if (std::holds_alternative<std::monostate>(record[i])) {
			if (fieldInfo.presence == FIELD_PRESENCE::REQUIRED) {
				throw std::runtime_error("Missing required data for field: " + fieldInfo.name + " at index: " + std::to_string(i) + "\n");
			} else { continue; }
		}
		std::visit([&](auto member, const auto& value) {
			using MemberType = std::remove_cvref_t<decltype(group.*member)>;
			using ValueType = std::remove_cvref_t<decltype(value)>;
			
			if constexpr (std::is_same_v<MemberType, ValueType>) {
				group.*member = value;
			} else if constexpr (std::is_integral_v<MemberType> && std::is_integral_v<ValueType>) { // Handles non-exact typing(needed for current dynamic width parsing)
				if constexpr (std::is_same_v<MemberType, bool>) {
					if (value == 0) {
						group.*member = false;
					} else if (value == 1) {
						group.*member = true;
					} else {
						throw std::runtime_error("Expected 1 or 0 for bool field.\n");
					}
				} else if (std::in_range<MemberType>(value)) {
					group.*member = static_cast<MemberType>(value);
				}
			}
			
			
			else {
				throw std::runtime_error("Type mismatch for field '" + fieldInfo.name + "': expected " + typeid(MemberType).name() + ", got " + typeid(ValueType).name());
			}
		}, groupMetadata[fieldMap[i]].ptr, record[i]);
	}


	size_t index = groups.size();
	groups.emplace_back(std::move(group));
	idMap.emplace(static_cast<GroupId>(groups[index].id), index);

}

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

std::vector<size_t> groupDB::validateSchema(const schemaData& schema) {
	return anime::domain::validateGroupSchema(schema);
}

recordField groupDB::getField(size_t id, size_t fieldIndex) const {
	size_t index = idMap.at(id);
	return getFromGroup(groups[index], fieldIndex);
}
recordField groupDB::getFromGroup(const Group& group, const size_t fieldIndex) const { // re-write, need to adapt recordField to work around fixed size types
	recordField result;

	const GroupMemberPtr& ptr = groupMetadata[fieldIndex].ptr;

	std::visit([&](auto memberPtr) {
		result = group.*memberPtr;
		}, ptr);

	return result;
}
}
