#include "group_compiler.hpp"
#include "group.hpp"
#include "../databases/entry_database.hpp"

#include <unordered_set>

namespace anime::domain {

void compileGroup(Group& group, const entryDB& entries) {
	for (const GroupField& field: groupMetadata) {
		if (!field.rule) { continue; } // skip if no compilation rule
		const auto& rule = *field.rule;

		std::visit([&](auto memberPtr) {
			auto accumulator = group.*memberPtr; // probably want to replace with a zeroed same type value
			if (rule.operation == COMPILE_OPERATION::MIN) {} // set to max value, else set to min, vectors are empty


			for (EntryId id : group.members) {
				if (rule.condition && !meetsCondition(entries, id, *rule.condition)) { continue; }
			
				compile(accumulator, entries, rule.operation) // accumulator is passed via reference
				
			}

			group.*memberPtr = accumulator;
		}, field.ptr);
	}
}

bool meetsCondition(const entryDB& entries, EntryId id, CompileCondition* condition) {
	auto entryField = entries.getField(id, entries.getFieldIndex(condition->field));

	return std::visit([&](auto referenceValue, auto entryValue) {
		if constexpr (!std::is_same_v(referenceValue, entryValue)) {
				throw std::runtime_error(" Compared fields arent the same type. \n");
		}

		switch (condition->type) {
			case CompileCondition::CONDITION_TYPE::EQUAL: {
				return entryValue == referenceValue;
			}
			case CompileCondition::CONDITION_TYPE::NOT_EQUAL: {
				return entryValue != referenceValue;
			}
			case CompileCondition::CONDITION_TYPE::GREATER_THAN: {
				return entryValue > referenceValue;
			}
			case CompileCondition::CONDITION_TYPE::LESS_THAN: {
				return entryValue < referenceValue;
			}
			case CompileCondition::CONDITION_TYPE::GREATER_EQUAL: {
				return entryValue >= referenceValue;
			}
			case CompileCondition::CONDITION_TYPE::LESS_EQUAL: {
				return entryValue =< referenceValue;
			}
			defualt:
				throw std::runtime_error("Invalid compile condition: "+ static_cast<uint8_t>(condition.type) + "\n");
		}

	}, condition->value, entryField);
}

void accumulate(int& accumulator, int entryField, COMPILE_OPERATION operation) {
	switch (operation) {
		case COMPILE_OPERATION::COUNT: {
			accumulator++;
			break;
		}
		case COMPILE_OPERATION::SUM: {
			accumulator + entryField;
			break;
		}
		case COMPILE_OPERATION::MIN: {
			accumulator = std::min(accumulator, entryField);
			break;
		}
		case COMPILE_OPERATION::MAX: {
			accumulator = std::max(accumulator, entryField);
			break;
		}
		case COMPILE_OPERATION::AVG: {
			throw std::runtime_error(" Scalar operation AVG is currently unsupported \n");
			break;
		}
		defualt:
			throw std::runtime_error(" Unsuported scalar operation: " + static_cast<uint8_t>(operation));
	}
}
void accumulate(std::vector<int>& accumulator, std::vector<int> entryField, COMPILE_OPERATION operation) {
	switch (operation) {
		case COMPILE_OPERATION::UNION: {
			std::unordered_set<int> buffer(accumulator.begin(), accumulator.end());

			buffer.insert_range(entryField.begin(), entryField.end());

			accumulator.clear();
			accumulator(buffer.begin(), buffer.end());

			std::sort(accumulator.begin(), accumulator.end(), std::greater<int>());

			break;
		}
		defualt:
			throw std::runtime_error("Unsuported vector operation: " + static_cast<uint8_t>(operation) + "\n");
	}
}

}
