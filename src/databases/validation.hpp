#pragma once

#include <unordered_map>
#include <unordered_set>
#include <ranges>
#include <algorithm>

#include "database_ops.hpp"
#include "schema/field.hpp"

namespace anime::storage::validation {

using anime::storage::helpers::always_false_v;
using anime::storage::helpers::is_vector;
using anime::storage::helpers::is_vector_v;
using anime::storage::helpers::UnsignedInteger;
using anime::storage::helpers::UnsignedVector;
using anime::storage::helpers::checkedConvert;


using anime::schema::FIELD_PRESENCE;
using anime::schema::VALUE_REFERENCE_TYPE;
using anime::schema::UPDATE_OPERATION;
using anime::schema::Bound;
using anime::schema::FieldInfo;
using anime::storage::schemaField;
using anime::storage::recordData;

enum class VALIDATION_SCOPE : uint8_t {
	ALL, // Checks all validation scopes
	ALL_LOCAL,
	IDS,                // checks exclusivness of IDs and index tables
	RANKS,              // checks continuity of ranks
	TITLES,             // checks exclusiveness of titles
	RANGES,             // checks that scalar fields are within valid ranges
	REFERENCES,         // checks that fields exist or are in range of their reference
	EXTERNAL_REFERENCES // Used with overloaded func. ex(check that entry group is in groupDB)
};

template <typename Database, typename External>
concept ValidatableDatabase = requires(const Database& database, const External& external) {
	{ database.validateIds() } -> std::same_as<bool>;
	{ database.validateRanks() } -> std::same_as<bool>;
	{ database.validateTitles() } -> std::same_as<bool>;
	{ database.validateRanges() } -> std::same_as<bool>;
	{ database.validateReferences() } -> std::same_as<bool>;
	{ database.validateExternal(external) } -> std::same_as<bool>;
};

// Primarily Post Validation after a Operation
// Validates a database in memory using a input scope
template <typename Database> bool validate(const Database& database, VALIDATION_SCOPE scope) {
	switch (scope) {
		case VALIDATION_SCOPE::ALL: {
			throw std::runtime_error(" Validation scope ALL requires external reference in arguments. ");
			break;
		}
		case VALIDATION_SCOPE::ALL_LOCAL: {
			return
				database.validateIds() &&
				database.validateRanks() &&
				database.validateTitles() &&
				database.validateRanges() &&
				database.validateReferences();
		}
		case VALIDATION_SCOPE::IDS: {
			return database.validateIds();
		}
		case VALIDATION_SCOPE::RANKS: {
			return database.validateRanks();
		}
		case VALIDATION_SCOPE::TITLES: {
			return database.validateTitles();
		}
		case VALIDATION_SCOPE::RANGES: {
			return database.validateRanges();
		}
		case VALIDATION_SCOPE::REFERENCES: {
			return database.validateReferences();
		}
		case VALIDATION_SCOPE::EXTERNAL_REFERENCES: {
			throw std::runtime_error(
				" Validation scope EXTERNAL_REFERENCES requires external reference in arguments. ");
			break;
		}
		default:
			throw std::runtime_error(" Invalid Validation Scope. ");
	}
	return false;
}
// Validates a database in memory using a input scope
template <typename Database, typename External> bool validate(const Database& database, VALIDATION_SCOPE scope, const External& external) {
	switch (scope) {
		case VALIDATION_SCOPE::ALL: {
			return validate(database, VALIDATION_SCOPE::ALL_LOCAL) && database.validateExternal(external);
		}
		case VALIDATION_SCOPE::ALL_LOCAL: {
			return validate(database, scope);
		}
		case VALIDATION_SCOPE::IDS: {
			return validate(database, scope);
		}
		case VALIDATION_SCOPE::RANKS: {
			return validate(database, scope);
		}
		case VALIDATION_SCOPE::TITLES: {
			return validate(database, scope);
		}
		case VALIDATION_SCOPE::RANGES: {
			return validate(database, scope);
		}
		case VALIDATION_SCOPE::REFERENCES: {
			return validate(database, scope);
		}
		case VALIDATION_SCOPE::EXTERNAL_REFERENCES: {
			return database.validateExternal(external);
		}
		default:
			throw std::runtime_error(" Invalid Validation Scope. ");
	}
	return false;
}
template <std::ranges::input_range Records, typename MemberPointer> bool validateIds(Records&& records, MemberPointer idField) {
	using IdType = std::remove_cvref_t<std::invoke_result_t<MemberPointer&, std::ranges::range_reference_t<Records>>>;
	std::unordered_set<IdType> seen;
	for (const auto& record : records) {
		if (!seen.insert(record.*idField).second) {
			return false;
		}
	}
	return true;
}
template <std::ranges::input_range Records, typename MemberPointer> bool validateRanks(Records&& records, MemberPointer rankField) {
	using RankType = std::remove_cvref_t<std::invoke_result_t<MemberPointer&, std::ranges::range_reference_t<Records>>>;
	auto non_zero = records | std::views::transform([rankField](const auto& record) {return record.*rankField;}) | std::views::filter([](RankType rank){ return rank != 0; });
	std::vector<RankType> ranks(non_zero.begin(), non_zero.end());
	if (ranks.empty()) {return true;}
	std::ranges::sort(ranks);
	if (ranks[0] == 1) {
		for (size_t i = 1; i < ranks.size(); ++i) {
			RankType diff = ranks[i] - ranks[i - 1];
			if (diff != 1)
				return false;
		}
	} else
		return false;
	return true;
}
template <std::ranges::input_range Records, typename MemberPointer> bool validateTitles(Records&& records, MemberPointer titleField) {
	std::unordered_set<std::string> seen;
	for (const auto& record : records) {
		if (!(record.*titleField).empty()) {
			if (!seen.insert(record.*titleField).second) {
				return false;
			}
		}
	}
	return true;
}
template <std::ranges::input_range Records, typename MemberPointer> bool validateRanges(Records&& records, MemberPointer rangedField, const Bound& bounds){
	using FieldType = std::remove_cvref_t<std::invoke_result_t<MemberPointer,std::ranges::range_value_t<Records>>>;
	if constexpr (UnsignedInteger<FieldType>) {
		for (const auto& record : records) {
			if (record.*rangedField > bounds.upper || record.*rangedField < bounds.lower) {
				return false;
			}
		}
	} else
		throw std::runtime_error(" Field with bounds must be a unsigned integer. ");
	return true;
}
template <std::ranges::input_range Records, typename MemberPointer> bool validateReferences(Records&& records, MemberPointer field, const std::vector<std::string>& lookupTable) {
	using FieldType = std::remove_cvref_t<std::invoke_result_t<MemberPointer&, std::ranges::range_reference_t<Records>>>;
	if constexpr (UnsignedInteger<FieldType>) {
		for (const auto& record : records) {
			if (record.*field >= lookupTable.size()) {
				return false;
			}
		}
	} else if constexpr (UnsignedVector<FieldType>) {
		for (const auto& record : records) {
			for (const auto& val : record.*field) {
				if (val >= lookupTable.size()) {
					return false;
				}
			}
		}
	} else {
		throw std::runtime_error(" Invalid field type for a reference table. ");
	}
	return true;
}
template <std::ranges::input_range Records, typename MemberPointer, typename Exists> bool validateExternal(Records&& records, MemberPointer field, Exists exists) {
	using FieldType = std::remove_cvref_t<std::invoke_result_t<MemberPointer&, std::ranges::range_reference_t<Records>>>;
	if constexpr (UnsignedInteger<FieldType>) {
		for (const auto& record : records) {
			if (!exists(record.*field)) {
				return false;
			}
		}
	} else if constexpr (UnsignedVector<FieldType>) {
		for (const auto& record : records) {
			for (const auto& val : record.*field) {
				if (!exists(val)) {
					return false;
				}
			}
		}
	} else {
		throw std::runtime_error(" Invalid field type for a reference table. ");
	}
	return true;
}


// Primarily pre/input validation
// Helpers for validating unsigned recordField alternatives.
template <std::unsigned_integral T> requires(!std::same_as<T, bool>)
void validateVectorElement(const FieldInfo& fieldInfo, const T& value) {
	switch (fieldInfo.type) {
		case schemaField::dataType::VECU8: {
			if (!std::in_range<std::uint8_t>(value)) {
				throw std::runtime_error(" Value to large for field, max value is: " + std::to_string(std::numeric_limits<std::uint8_t>::max()) +", Got:" + std::to_string(value));
			}
			break;
		}
		case schemaField::dataType::VECU16: {
			if (!std::in_range<std::uint16_t>(value)) {
				throw std::runtime_error(" Value to large for field, max value is: " + std::to_string(std::numeric_limits<std::uint16_t>::max()) + ", Got:" + std::to_string(value));
			}
			break;
		}
		case schemaField::dataType::VECU32: {
			if (!std::in_range<std::uint32_t>(value)) {
				throw std::runtime_error(" Value to large for field, max value is: " + std::to_string(std::numeric_limits<std::uint32_t>::max()) + ", Got:" + std::to_string(value));
			}
			break;
		}
		case schemaField::dataType::VECU64: {
			if (!std::in_range<std::uint64_t>(value)) {
				throw std::runtime_error(" Value to large for field, max value is: " + std::to_string(std::numeric_limits<std::uint64_t>::max()) + ", Got:" + std::to_string(value));
			}
			break;
		}
		default:
			throw std::runtime_error(" Invalid unsigned integer type. ");
	}
}
// Overloads for validating recordField alternatives and the variant itself.
template <std::unsigned_integral T> requires(!std::same_as<T, bool>)
void validateField(const FieldInfo& fieldInfo, const T& value) {
	switch (fieldInfo.type) {
		case schemaField::dataType::U8: {
			if (!std::in_range<std::uint8_t>(value)) {
				throw std::runtime_error(" Value to large for field, max value is: " + std::to_string(std::numeric_limits<std::uint8_t>::max()) + ", Got:" + std::to_string(value));
			}
			break;
		}
		case schemaField::dataType::U16: {
			if (!std::in_range<std::uint16_t>(value)) {
				throw std::runtime_error(" Value to large for field, max value is: " + std::to_string(std::numeric_limits<std::uint16_t>::max()) + ", Got:" + std::to_string(value));
			}
			break;
		}
		case schemaField::dataType::U32: {
			if (!std::in_range<std::uint32_t>(value)) {
				throw std::runtime_error(" Value to large for field, max value is: " + std::to_string(std::numeric_limits<std::uint32_t>::max()) + ", Got:" + std::to_string(value));
			}
			break;
		}
		case schemaField::dataType::U64: {
			if (!std::in_range<std::uint64_t>(value)) {
				throw std::runtime_error(" Value to large for field, max value is: " + std::to_string(std::numeric_limits<std::uint64_t>::max()) + ", Got:" + std::to_string(value));
			}
			break;
		}
		default:
			throw std::runtime_error(" Invalid unsigned integer type. ");
	}
	switch (fieldInfo.reference) {
		case VALUE_REFERENCE_TYPE::LOOKUP_TABLE: {
			break;
		}
		case VALUE_REFERENCE_TYPE::BOUNDS: {
			if (!fieldInfo.bounds.has_value()) {
				throw std::runtime_error("Missing bounds for range field: " + fieldInfo.name);
			}
			if (value < fieldInfo.bounds->lower || value > fieldInfo.bounds->upper) {
				throw std::runtime_error("Value for field \"" + fieldInfo.name + "\" out of specified range.");
			}
			break;
		}
		case VALUE_REFERENCE_TYPE::NONE: {
			break;
		}
		default:
			throw std::runtime_error("Unknown value reference type for field: " + fieldInfo.name);
	}
}
template <std::unsigned_integral T> requires(!std::same_as<T, bool>)
void validateField(const FieldInfo& fieldInfo, const std::vector<T>& value) {
	if (value.empty()) {
		return;
	}

	auto max = std::max_element(value.begin(), value.end());
	auto max_value = *max;
	size_t index = std::distance(value.begin(), max);
	try {
		validateVectorElement(fieldInfo, max_value);
	} catch (const std::exception& e) {
		throw std::runtime_error(std::string(e.what()) + ", at input vector index: " + std::to_string(index));
	}
}
inline void validateField(const FieldInfo& fieldInfo, bool value) {
	if (fieldInfo.type != schemaField::dataType::BOOL) {
		throw std::runtime_error(" Field does not expect a boolean value. ");
	}
}
inline void validateField(const FieldInfo& fieldInfo, const std::string& value) {
	if (fieldInfo.type != schemaField::dataType::STR) {
		throw std::runtime_error(" Field does not expect a string value. ");
	}
}
inline void validateField(const FieldInfo& fieldInfo, const std::monostate value) {
	return;
}
inline void validateField(const FieldInfo& fieldInfo, const recordField& record) {
	std::visit([&](const auto& value) { validateField(fieldInfo, value); }, record);
}





}

