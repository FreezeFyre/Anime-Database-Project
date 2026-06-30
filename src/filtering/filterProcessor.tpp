#pragma once

template <FilterableDatabase Database>
auto filterProcessor::query(const FilterRequest& request, const Database& database) -> std::vector<typename Database::IdType> {
	ValidatedFilters validatedFilters;

	for (const FieldFilter& filter : request) {
		if (!hasFilter(filter)) {
			continue;
		}

		size_t index = 0;
		try {
			index = database.getFieldIndex(filter.name);
		} catch (const std::exception& e) {
			std::cerr << "Error retrieving field index for filter \"" << filter.name << "\": " << e.what() << "\n";
			throw; // Rethrow to allow caller to handle as well
		}

		const auto& info = database.getFieldInfo(index);

		try {
			validateFilter(filter, info);
		} catch (const std::exception& e) {
			std::cerr << "Error validating filter for field \"" << filter.name << "\": " << e.what() << "\n";
			throw; // Rethrow to allow caller to handle as well
		}

		validatedFilters.push_back({index, &filter});

	}

	std::vector<typename Database::IdType> result;


	for (size_t i = 0; i < database.getSize(); i++) {
		bool passes = true;

		for (const ValidatedFilter& validatedFilter : validatedFilters) {
			recordField field = database.getField(i, validatedFilter.fieldIndex);

			const auto& info = database.getFieldInfo(validatedFilter.fieldIndex);
			if (!check(*validatedFilter.filter, field, info)) {
				passes = false;
				break;
			}
		}

		if (passes) {
			result.push_back(database.getId(i));
		}
	}
	return result;
}
