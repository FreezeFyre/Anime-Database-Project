#pragma once

#include "schema/field.hpp"

#include <algorithm>
#include <concepts>
#include <optional>
#include <string>

#include <iterator>
#include <limits>
#include <type_traits>
#include <utility>

#include <cstdint>
#include <exception>
#include <io/file_types.hpp>
#include <stdexcept>
#include <variant>
#include <vector>
#include <unordered_set>
#include <ranges>

namespace anime::storage::helpers {

using anime::schema::FieldInfo;
using anime::schema::VALUE_REFERENCE_TYPE;
using anime::storage::schemaField;
using anime::storage::recordData;

template <typename> inline constexpr bool always_false_v = false;
template <typename T> struct is_vector : std::false_type {};
template <typename T, typename Allocator> struct is_vector<std::vector<T, Allocator>> : std::true_type {};
template <typename T> inline constexpr bool is_vector_v = is_vector<std::remove_cvref_t<T>>::value;
template <typename T> concept UnsignedInteger = std::unsigned_integral<std::remove_cvref_t<T>> && !std::same_as<std::remove_cvref_t<T>, bool>;
template <typename T> concept UnsignedVector = is_vector_v<T> && UnsignedInteger<typename std::remove_cvref_t<T>::value_type>;


// Should add a helper that tests for compatible typing, ie vector_uint8 is compatible to vector_uint64 but not vice versa etc etc

// Helpers for converting unsigned recordField alternatives.
template <UnsignedInteger Destination, UnsignedInteger Source>
Destination checkedConvert(const Source input) {
	if (std::in_range<Destination>(input)) {
		return static_cast<Destination>(input);
	}
	else {
		throw std::runtime_error(" Input value is not within the range of the provided type. ");
	}
}

} // namespace anime::storage::helpers

namespace anime::storage::operators {

using helpers::is_vector_v;
using helpers::UnsignedInteger;
using helpers::checkedConvert;


namespace vector_ops {
	// Overloads for adding unsigned scalar or vector recordField values uniquely.
	template <UnsignedInteger T> void addUniqueElement(std::vector<T>& field, const recordField& value) {
		std::visit(
			[&](const auto& input) {
				using InputType = std::remove_cvref_t<decltype(input)>;

				if constexpr (!is_vector_v<InputType>) {
					std::vector<T> buffer;
					for (const auto& element : input) {
						if (!std::ranges::contains(buffer, element)) {
							buffer.push_back(checkedConvert<T>(element));
						}
					}
					field.append_range(buffer);
				} else if constexpr (UnsignedInteger<InputType>) {

				} else {
					throw std::runtime_error(" Input value cannot be a monostate or string and must be integral");
				}
			},
			value);
	}
	template <UnsignedInteger T, UnsignedInteger U> void addUniqueElement(std::vector<T>& field, const U& input) {
		if (!std::ranges::contains(field, input)) {
			field.push_back(checkedConvert<T>(input));
		}
	}
	template <UnsignedInteger T, UnsignedInteger U> void addUniqueElement(std::vector<T>& field, const std::vector<U>& input) {
		std::vector<T> buffer = field;
		for (const auto& element : input) {
			if (!std::ranges::contains(buffer, element)) {
				buffer.push_back(checkedConvert<T>(element));
			}
		}
		field = std::move(buffer);
	}
	template <UnsignedInteger T, typename U> void addUniqueElement(std::vector<T>& field, const U& input) {
		throw std::runtime_error(" Add Unique Element expects a input of a Unsigned Integer or a vector of. ");
	}


	// Overloads for removing unsigned scalar or vector recordField values.
	template <UnsignedInteger T> void removeElement(std::vector<T>& field, const recordField& value) {
		std::visit(
			[&](const auto& input) {
				using InputType = std::remove_cvref_t<decltype(input)>;

				if constexpr (is_vector_v<InputType>) {
					using ElementType = std::ranges::range_value_t<InputType>;
					if constexpr (UnsignedInteger<ElementType>) {
						auto buffer = field;

						std::unordered_set<ElementType> removals(input.begin(), input.end());
						std::erase_if(buffer, [&](T x) { return removals.contains(x); });

						field = buffer;
					} else {
						throw std::runtime_error(" Target vector and input vector must hold compatible types.");
					}
				} else if constexpr (UnsignedInteger<InputType>) {
					auto it = std::ranges::find(field, input);

					if (it != field.end()) {
						field.erase(it);
					}
				} else {
					throw std::runtime_error(" Input value cannot be a monostate or string and must be integral");
				}
			},
			value);
	}
	template <UnsignedInteger T, UnsignedInteger U> void removeElement(std::vector<T>& field, const U& input) {
		auto it = std::ranges::find(field, input);

		if (it != field.end()) {
			field.erase(it);
		}
	}
	template <UnsignedInteger T, UnsignedInteger U> void removeElement(std::vector<T>& field, const std::vector<U>& input) {
		auto buffer = field;

		std::unordered_set<U> removals(input.begin(), input.end());
		std::erase_if(buffer, [&](T x) { return removals.contains(x); });

		field = std::move(buffer);
	}
	template <UnsignedInteger T, typename U> void removeElement(std::vector<T>& field, const U& input) {
		throw std::runtime_error(" Remove Element expects a input of a Unsigned Integer or a vector of. ");
	}
}

namespace field_ops {
	// Overloads for assigning recordField values or their alternatives to matching field types.
	template <typename T> void setField(T& field, const recordField& value) {
		// currently using primitive approach
		// Check if types are the same/compatible and if they are set(primitive is just is_same_v<> but more complex
		// would be checking ranges/values for integers and vectors of)
		std::visit(
			[&](const auto& input) {
				using InputType = std::remove_cvref_t<decltype(input)>;
				if constexpr (std::same_as<T, InputType>) {
					field = input;
				} else {
					throw std::runtime_error(" Provided value must be of the same type as the target field. ");
				}
			},
			value);
	}
	template <typename T, typename U> void setField(T& field, const U& input) {
		if constexpr (std::same_as<T, U>) {
			field = input;
		} else {
			throw std::runtime_error(" Provided value must be of the same type as the target field. ");
		}
	}
	// Helper for clearing fields whose types match non-monostate recordField alternatives.
	template <typename T> void clearField(T& field) {
		// Need to decide how to approach, im unsure if i can use native type defualts, string should be empty string,
		// vector empty vector and any number should be to 0
		if constexpr (is_vector_v<T>) {
			field.clear();
		} else if constexpr (UnsignedInteger<T>) {
			field = 0;
		} else if constexpr (std::same_as<T, std::string>) {
			field = "";
		} else if constexpr (std::same_as<T, bool>) {
			field = false;
		}
	}
}


} // namespace anime::storage::operators
