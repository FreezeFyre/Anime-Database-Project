#pragma once

#include "domain/group.hpp"
#include "databases/entry_database.hpp"
#include <span>

namespace anime::domain {

using anime::storage::entryDB;

void compileGroup(Group& group, const entryDB& entries);

bool meetsCondition(const entryDB& entries, EntryId id, CompileCondition* condition);

template <typename T> void compile(T& accumulator, const entryDB& entries, COMPILE_OPERATION operation) {
	auto entryField = entries.getField(id, entries.getFieldIndex(rule.source));
	std::visit([&](auto entryFieldValue) {
	
		// here i call my operation logic
		accumulate(accumulator, entryFieldValue, operation);


	}, entryField);
}

// Temporarily non-fixed size for simplicity so i dont have to start implemnenting my refactor to global fixed sizes
void accumulate(int& accumulator, int entryField, COMPILE_OPERATION operation);
void accumulate(std::vector<int>& accumulator, std::vector<int> entryField, COMPILE_OPERATION operation);



}
