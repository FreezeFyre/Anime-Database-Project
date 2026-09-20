#pragma once

#include "domain/types.hpp"
#include "schema/field.hpp"

#include <algorithm>
#include <cctype>
#include <concepts>
#include <optional>
#include <string>

#include <iterator>
#include <limits>
#include <type_traits>
#include <utility>

#include <charconv>
#include <cstdint>
#include <fstream>
#include <ostream>
#include <stdexcept>
#include <string_view>
#include <system_error>
#include <variant>
#include <vector>

namespace anime::storage::helpers {

using anime::schema::FieldInfo;
using anime::storage::schemaField;
using anime::storage::recordData;

template <typename T> struct is_vector : std::false_type {};
template <typename T, typename Allocator> struct is_vector<std::vector<T, Allocator>> : std::true_type {};

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
void validateField(const FieldInfo& fieldInfo, bool value);
void validateField(const FieldInfo& fieldInfo, const std::string& value);
void validateField(const FieldInfo& fieldInfo, const std::monostate value);
void validateField(const FieldInfo& fieldInfo, const recordField& record);

} // namespace anime::storage::helpers

namespace anime::storage::operators {


}
