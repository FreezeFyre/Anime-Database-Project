#include "domain/entry_schema.hpp"
#include <iostream>
#include <unordered_set>

namespace anime::domain {
using anime::storage::schemaData;

std::vector<size_t> validateEntrySchema(const schemaData& schema) {
	std::vector<size_t> fieldIndices;
	std::unordered_set<std::string> seen;

	for (size_t i = 0; i < schema.size(); i++) {
		const schemaField& field = schema[i];

		if (!seen.insert(field.name).second) {
			std::cerr << "Duplicate schema field: \"" << field.name
				<< "\" at index: " << i << "\n";
			throw std::runtime_error("Duplicate schema field: " + field.name);
		}

		std::unordered_map<std::string, size_t>::const_iterator found = entryFieldNames.find(field.name);

		if (found == entryFieldNames.end()) {
			std::cerr << "Schema field: \"" << field.name
				<< "\" at index: " << i
				<< " not in EntryMeta.\n";
			throw std::runtime_error("Schema field not found in EntryMeta: " + field.name);
		}

		size_t metaIndex = found->second;

		if (entryMetadata[metaIndex].info.type != field.type) {
			std::cerr << "Schema field: \"" << field.name
				<< "\" at index: " << i
				<< " has incorrect data type.\n";
			throw std::runtime_error("Schema field has incorrect data type: " + field.name);
		}

		fieldIndices.push_back(metaIndex);
	}

	for (size_t i = 0; i < entryMetadata.size(); i++) {
		const FieldInfo& meta = entryMetadata[i].info;

		if (!seen.contains(meta.name)) {
			if (meta.presence == FIELD_PRESENCE::REQUIRED) {
				std::cerr << "Required field \"" << meta.name
					<< "\" is missing from schema.\n";
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
} // namespace anime::domain
