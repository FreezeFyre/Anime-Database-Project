#include "group_schema.hpp"

#include <unordered_set>
#include <iostream>

namespace anime::domain {
using anime::storage::schemaData;


std::vector<size_t> validateGroupSchema(const schemaData& schema) {
	std::vector<size_t> fieldIndices;
	std::unordered_set<std::string> seen;

	for (const schemaField& field : schema) {
		if (!seen.insert(field.name).second) {
			throw std::runtime_error("Duplicate schema field: " + field.name);
		}
		std::unordered_map<std::string, size_t>::const_iterator found = groupFieldNames.find(field.name);

		if (found == groupFieldNames.end()) {
			throw std::runtime_error("Schema field not found in GroupMeta: " + field.name);
		}
		size_t metaIndex = found->second;

		if (groupMetadata[metaIndex].info.type != field.type) {
			throw std::runtime_error("Schema field has incorrect data type: " + field.name);
		}

		fieldIndices.push_back(metaIndex);
	}

	for (const GroupField& metaField : groupMetadata) {
		const auto& meta = metaField.info;
		if (!seen.contains(meta.name)) {
			if (meta.presence == FIELD_PRESENCE::REQUIRED) {
				throw std::runtime_error("Required field is missing from schema: " + meta.name);
			}
			else if (meta.presence == FIELD_PRESENCE::RECOMMENDED) {
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
}
