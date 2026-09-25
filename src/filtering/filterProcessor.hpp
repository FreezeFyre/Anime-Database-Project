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
#include <array>
#include <optional>
#include <span>
#include <concepts>
#include <string_view>

#include "io/fileHandling.hpp"
#include "domain/types.hpp"
#include "schema/field.hpp"
#include "net/protocol/data.hpp"

namespace anime::filtering {

using anime::schema::FieldInfo;
using anime::storage::schemaField;
using anime::storage::recordField;
using anime::protocol::ByteReader;
using FilterValue = uint32_t;

struct Range {
	std::optional<FilterValue> min;
	std::optional<FilterValue> max;
};

struct FieldFilter {
	std::string name;
	std::vector<FilterValue> includeAll;
	std::vector<FilterValue> includeAny;
	std::vector<FilterValue> exclude;
	Range range;
};


using FilterRequest = std::vector<FieldFilter>;

struct ValidatedFilter {
	size_t fieldIndex;
	const FieldFilter* filter;
};

using ValidatedFilters = std::vector<ValidatedFilter>;

template<typename Database>
concept FilterableDatabase = requires(const Database& db, size_t index, const std::string& fieldName) {
	typename Database::IdType;
	{ db.getSize() } -> std::convertible_to<size_t>;
	{ db.getFieldCount() } -> std::convertible_to<size_t>;
	{ db.getFieldIndex(fieldName) } -> std::same_as<size_t>;
	{ db.getFieldInfo(index).mode } -> std::convertible_to<schemaField::filterMode>;
	{ db.getField(index, index) } -> std::same_as<recordField>;
	{ db.getId(index) } -> std::same_as<typename Database::IdType>;
};


class filterProcessor {
	public:
		template <FilterableDatabase Database>
		auto query(const FilterRequest& request, const Database& database)
		-> std::vector<typename Database::IdType>;

		FilterRequest decode(ByteReader& reader);
	private:
		bool checkRange(const Range& r, size_t value);

		bool includesAny(const std::vector<size_t>& vec, size_t value);
		bool includesAny(const std::vector<FilterValue>& vec, FilterValue value);
		bool includesAny(const std::vector<size_t>& vec, const std::vector<size_t>& value);
		bool includesAny(const std::vector<FilterValue>& vec, const std::vector<int>& value);

		bool includesAll(const std::vector<size_t>& reference, const std::vector<size_t>& values);
		bool includesAll(const std::vector<FilterValue>& reference, const std::vector<int>& values);

		bool hasFilter(const FieldFilter& filter);

		bool validateFilter(const FieldFilter& filter, const FieldInfo& info);

		bool check(const FieldFilter& filter, const recordField& field, const FieldInfo& info);
};


#include "filtering/filterProcessor.tpp"

} // namespace anime::filtering
