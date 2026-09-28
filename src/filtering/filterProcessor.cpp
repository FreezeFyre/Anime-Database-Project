#include "filtering/filterProcessor.hpp"

namespace anime::filtering {

bool filterProcessor::checkRange(const Range& r, size_t value) {
	FilterValue filterValue = static_cast<FilterValue>(value);
	if (r.max and filterValue > *r.max) {
		return false;
	}
	if (r.min and filterValue < *r.min) {
		return false;
	}
	return true;
}




bool filterProcessor::includesAny(const std::vector<size_t>& vec, size_t value) {
	for (size_t i = 0; i < vec.size(); i++) {
		if (vec[i] == value) {
			return true;
		}
	}
	return false;
}

bool filterProcessor::includesAny(const std::vector<FilterValue>& vec, FilterValue value) {
	/*


	I can probably replace the loop and all its logic with a std::find


	*/
	for (size_t i = 0; i < vec.size(); i++) {
		if (vec[i] == value) {
			return true;
		}
	}
	return false;
}


bool filterProcessor::includesAny(const std::vector<size_t>& vec, const std::vector<size_t>& value) {
	for (size_t i = 0; i < value.size(); i++) {
		for (size_t n = 0; n < vec.size(); n++) {
			if (value[i] == vec[n])
			{
				return true;
			}
		}
	}
	return false;
}

bool filterProcessor::includesAny(const std::vector<FilterValue>& vec, const std::vector<int>& value) {
	/*
	
	
	May want to replace with a unordered_set and use find to scale for large vectors
	and avoid O(n^2) time and get O(n) build and O(1) lookup
	
	
	*/
	for (size_t i = 0; i < value.size(); i++) {
		for (size_t n = 0; n < vec.size(); n++) {
			if (static_cast<FilterValue>(value[i]) == vec[n])
			{
				return true;
			}
		}
	}
	return false;
}


bool filterProcessor::includesAll(const std::vector<size_t>& reference, const std::vector<size_t>& values) {
	for (size_t i = 0; i < reference.size(); i++) {
		if (!includesAny(values, reference[i])) {
			return false;
		}
	}
	return true;
}

bool filterProcessor::includesAll(const std::vector<FilterValue>& reference, const std::vector<int>& values) {
	/*
	
	
	May want to replace with a unordered_set and use find to scale for large vectors
	and avoid O(n^2) time and get O(n) build and O(1) lookup
	
	
	*/
	for (size_t i = 0; i < reference.size(); i++) {
		bool found = false;
		for (size_t n = 0; n < values.size(); n++) {
			if (reference[i] == static_cast<FilterValue>(values[n])) {
				found = true;
				break;
			}
		}

		if (!found) {
			return false;
		}
	}
	return true;
}

bool filterProcessor::hasFilter(const FieldFilter& filter) {
	return
		!filter.includeAll.empty() ||
		!filter.includeAny.empty() ||
		!filter.exclude.empty() ||
		filter.range.min.has_value() ||
		filter.range.max.has_value();
}


bool filterProcessor::validateFilter(const FieldFilter& filter, const FieldInfo& info) {
	if (filter.includeAll.empty() and filter.includeAny.empty() and filter.exclude.empty() and !filter.range.min.has_value() and !filter.range.max.has_value()) {
		std::cerr << "No valid filter criteria provided for field: \"" << filter.name << "\"\n";
		return false;
	}

	switch (info.mode) {
		case schemaField::filterMode::FM_RANGE: {
			if (filter.range.min.has_value() and filter.range.max.has_value() and filter.range.min > filter.range.max) {
				std::cerr << "Invalid range filter for field \"" << filter.name << "\": min is greater than max.\n";
				return false;
			}
			break;
		}
		case schemaField::filterMode::FM_MULTI_VALUE: {
			if (filter.includeAll.empty() and filter.includeAny.empty()) {
				std::cerr << "Multi-value filter for field \"" << filter.name << "\" must have at least one includeAll or includeAny value.\n";
				return false;
			}
			break;
		}
		case schemaField::filterMode::FM_RMV: {
			if (filter.includeAny.empty()) {
				std::cerr << "RMV filter for field \"" << filter.name << "\" must have at least one includeAny value.\n";
				return false;
			}
			break;
		}
		default:
			std::cerr << "Unknown or unsupported filter mode for field \"" << filter.name << "\".\n";
			return false;
	}

	return true;
}

bool filterProcessor::check(const FieldFilter& filter, const recordField& field, const FieldInfo& info) {
	using anime::storage::helpers::UnsignedInteger;
	using anime::storage::helpers::UnsignedVector;

	return std::visit( [&](const auto& value) {
		using FieldType = std::remove_cvref_t<decltype(value)>;
		switch (info.mode) {
			case schemaField::filterMode::FM_RANGE: {
				if constexpr (UnsignedInteger<FieldType>) {
					if (checkRange(filter.range, value)) {
						return true;
					}
				}
				return false;
			}
			case schemaField::filterMode::FM_MULTI_VALUE: {
				if constexpr (UnsignedInteger<FieldType>) {
					if (!filter.exclude.empty() and std::ranges::contains(filter.exclude, value)) {
						return false;
					}
					if (!filter.includeAny.empty() && !std::ranges::contains(filter.includeAny, value)) {
						return false;
					}
					if (!filter.includeAll.empty() && !std::ranges::contains(filter.includeAll, value)) {
						return false;
					}
					return true;
				} else if constexpr (UnsignedVector<FieldType>) {
					if (!filter.exclude.empty() and std::ranges::any_of(filter.exclude, [&](const auto& v) { return std::ranges::contains(value, v); })) {
						return false;
					}
					if (!filter.includeAny.empty() and !std::ranges::any_of(filter.includeAny, [&](const auto& v) { return std::ranges::contains(value, v); })) {
						return false;
					}
					if (!filter.includeAll.empty() and !std::ranges::all_of(filter.includeAll, [&](const auto& v) { return std::ranges::contains(value, v); })) {
						return false;
					}
					return true;
				} else
					throw std::runtime_error(" Currently unsupported type for filtering");
			}
			case schemaField::filterMode::FM_RMV: {
				if constexpr(!UnsignedInteger<FieldType>) {
					return false;
				}
				else if (!checkRange(filter.range, value) ||
						 (!filter.includeAny.empty() && !std::ranges::contains(filter.includeAny, value)) ||
						 (!filter.exclude.empty() && std::ranges::contains(filter.exclude, value))
						) {
					return false;
				}
				return true;
			}
			default: 
				throw std::runtime_error(" Unsupported filter mdoe. ");
		}
	}, field);
}

namespace {
constexpr uint8_t HAS_INCLUDE_ALL = 1 << 0;
constexpr uint8_t HAS_INCLUDE_ANY = 1 << 1;
constexpr uint8_t HAS_EXCLUDE     = 1 << 2;
constexpr uint8_t HAS_RANGE_MIN   = 1 << 3;
constexpr uint8_t HAS_RANGE_MAX   = 1 << 4;
} // namespace

FilterRequest filterProcessor::decode(ByteReader& r) {
	FilterRequest result;
	size_t filterCount = r.read<std::uint32_t>();

	for (size_t i = 0; i < filterCount; i++) {
		FieldFilter filter;
		filter.name = r.read<std::string>();

		uint8_t flags = r.read<std::uint8_t>();

		if (flags & HAS_INCLUDE_ALL) {
			filter.includeAll = r.read<std::vector<std::uint32_t>>();
		}

		if (flags & HAS_INCLUDE_ANY) {
			filter.includeAny = r.read<std::vector<std::uint32_t>>();
		}

		if (flags & HAS_EXCLUDE) {
			filter.exclude = r.read<std::vector<std::uint32_t>>();
		}

		if (flags & HAS_RANGE_MIN) {
			filter.range.min = r.read<std::uint32_t>();
		}

		if (flags & HAS_RANGE_MAX) {
			filter.range.max = r.read<std::uint32_t>();
		}

		result.push_back(std::move(filter));
	}

	return result;
}

} // namespace anime::filtering
