#pragma once



template<typename f>
std::vector<size_t> filterProcessor::checkFilters(const indexFilters& filters, const f& database) {

	std::span<const fieldData> meta = database.getMeta();
	std::vector<size_t> result;
	
	for (size_t i = 0; i < database.getSize(); i++) {

		bool passes = true;

		for (size_t n = 0; n < meta.size(); n++) {

			if (!hasFilter(filters[n])) {
				continue;
			}

			recordField field = database.getField(i, n);


			switch (meta[n].mode) {
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
			result.push_back(i);
		}

	}

	return result;
}



