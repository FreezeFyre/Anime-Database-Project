#pragma once



template<typename f>
std::vector<size_t> filterProcessor::checkFilters(const indexFilters& filters, f& database) {
	std::vector<size_t> result;

	schemaData schema = database.getSchema();
	
	for (size_t i = 0; i < database.getSize(); i++) {

		bool passes = true;

		for (size_t n = 0; n < database.getFieldCount(); n++) {

			if (!hasFilter(filters[n])) {
				continue;
			}

			recordField field = database.getField(i, n);


			switch (schema[n].mode) {
				case schemaField::filterMode::FM_RANGE: {
					if (!checkRange(filters[n].bounds, std::get<int>(field))) {
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
					if (!checkRange(filters[n].bounds, std::get<int>(field)) or !includesAny(filters[n].includeAny, std::get<int>(field))) {
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
			result.push_back(i);
		}

	}

	return result;
}



