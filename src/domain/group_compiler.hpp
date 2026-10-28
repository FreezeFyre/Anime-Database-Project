#pragma once

#include "domain/group.hpp"
#include "databases/entry_database.hpp"
#include <span>

#include <execution>
#include <algorithm>

namespace anime::domain::group {
 
using anime::storage::entryDB;
using anime::storage::helpers::UnsignedInteger;
using anime::storage::helpers::UnsignedVector;
using anime::storage::helpers::BothIntegral;
using anime::storage::helpers::is_vector_v;

using anime::storage::operators::field_ops::clearField;

void compileGroup(Group& group, const entryDB& entries);

bool meetsCondition(const entryDB& entries, EntryId id, const CompileCondition& condition);

template <typename Record, UnsignedInteger Field, UnsignedInteger Addend> void addUnsigned(Record& record, Field Record::* memberPtr, Addend addend) {
	// Throwing Func | Tests if addend value can fit into the fields data type and then returns the sum
	auto& field = record.*memberPtr;
	if (addend > (std::numeric_limits<Field>::max)() - field) {
		throw std::overflow_error(" Sum exceeds the target fields range. ");
	}
	field += static_cast<Field>(addend);
}
template <UnsignedInteger Field, UnsignedInteger Addend> void addUnsigned(Field& field, Addend addend) {
	// Throwing Func | Tests if addend value can fit into the fields data type and then returns the sum
	if (addend > (std::numeric_limits<Field>::max)() - field) {
		throw std::overflow_error(" Sum exceeds the target fields range. ");
	}
	field += static_cast<Field>(addend);
}

template <typename T> T prepField(const Group& group, T Group::*memberPtr, COMPILE_OPERATION operation) {
	T data = group.*memberPtr;
	clearField(data);
	return data;
}

template <UnsignedInteger T, UnsignedInteger U> void accumulate(T& field, const U& entryField, COMPILE_OPERATION operation) {
	switch (operation) {
		case COMPILE_OPERATION::COUNT: {
			if (field == (std::numeric_limits<T>::max)()) {
				throw std::runtime_error(" Cant iterate, Field already holds max value. ");
			}
			field++;
			break;
		}
		case COMPILE_OPERATION::SUM: {
			addUnsigned(field, entryField);
			break;
		}
		case COMPILE_OPERATION::MIN: {
			if (!std::in_range<T>(entryField)) {
				throw std::runtime_error(" Input value is out of the range of the group field's type. ");
			}
			auto min = std::min(field, static_cast<T>(entryField));
			field = min;
			break;
		}
		case COMPILE_OPERATION::MAX: {
			if (!std::in_range<T>(entryField)) {
				throw std::runtime_error(" Input value is out of the range of the group field's type. ");
			}
			auto max = std::max(field, static_cast<T>(entryField));
			field = max;
			break;
		}
		case COMPILE_OPERATION::AVG: {
			throw std::runtime_error(" Scalar operation AVG is currently unsupported. ");
			break;
		}
		default:
			throw std::runtime_error(" Unsuported scalar operation: " + std::to_string(static_cast<uint8_t>(operation)));
	}
}
template <UnsignedInteger T, UnsignedInteger U> void accumulate(std::vector<T>& field, const std::vector<U>& entryField, COMPILE_OPERATION operation) {
	switch (operation) {
		case COMPILE_OPERATION::UNION: {
			std::unordered_set<T> buffer(field.begin(), field.end());

			const bool fits = std::all_of(std::execution::par_unseq,entryField.begin(), entryField.end(), [&](const auto& value) { return std::in_range<T>(value); });
			if (!fits) {
				throw std::runtime_error(" Not all values from the input vector fit into the type of the group fields vector. ");
			}
			buffer.insert_range(entryField);

			field.clear();
			std::vector<T> temp(buffer.begin(), buffer.end());
			field = temp;

			std::sort(field.begin(), field.end(), std::greater<T>());

			break;
		}
		default:
			throw std::runtime_error("Unsuported vector operation: " + std::to_string(static_cast<uint8_t>(operation)) + "\n");
	}
}
template <typename T, typename U>
void accumulate(T&, const U&, COMPILE_OPERATION) {
	throw std::runtime_error("Unsupported accumulation types.");
}


}



