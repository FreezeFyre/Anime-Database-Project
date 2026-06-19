#pragma once



template<typename Database>
auto filterProcessor::checkFilters(const indexFilters& filters, const Database& database) -> std::vector<typename Database::IdType> {

	std::vector<typename Database::IdType> result;
	
	for (size_t i = 0; i < database.getSize(); i++) {

		bool passes = true;

		for (size_t n = 0; n < database.getFieldCount(); n++) {

			if (n >= filters.size() or !hasFilter(filters[n])) {
				continue;
			}

			recordField field = database.getField(i, n);

			const auto& info = database.getFieldInfo(n);
			switch (info.mode) {
				case schemaField::filterMode::FM_RANGE: {
					if (!checkRange(filters[n].range, std::get<int>(field))) {
						passes = false;
					}
					break;
				}
				case schemaField::filterMode::FM_MULTI_VALUE: {
					if (!includesAny(filters[n].includeAny, std::get<int>(field))) {
						passes = false;
					}
					break;
				}
				case schemaField::filterMode::FM_RMV: {
					if (!checkRange(filters[n].range, std::get<int>(field)) or !includesAny(filters[n].includeAny, std::get<int>(field))) {
						passes = false;
					}
					break;
				}
				default:
					// No Filter or Unknown Filter Mode
					break;
			}

		}
		if (passes) {
			result.push_back(database.getId(i));
		}

	}

	return result;
}



