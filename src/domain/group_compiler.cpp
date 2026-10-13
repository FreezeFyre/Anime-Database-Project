#include "group_compiler.hpp"
#include "group.hpp"
#include "../databases/entry_database.hpp"

#include <unordered_set>

namespace anime::domain::group {

void compileGroup(Group& group, const entryDB& entries) {
	for (const GroupField& field : groupMetadata) {
		if (!field.rule) {
			continue;
		} // skip if no compilation rule
		const auto& rule = *field.rule;

		std::visit( [&](auto memberPtr) {
			auto value = group.*memberPtr; // probably want to replace with a zeroed same type value
			if constexpr (UnsignedInteger<decltype(value)>)	{
				clearField(value);
				if (rule.operation == COMPILE_OPERATION::MIN) {
					value = std::numeric_limits<decltype(value)>::max();
				} // set to max value, else set to min, vectors are empty

				for (EntryId id : group.members) {
					if (rule.condition && !meetsCondition(entries, id, *rule.condition)) {
						continue;
					}
					recordField entryField = entries.getField(id, entries.getFieldIndex(*rule.source));
					compile(value, entryField, rule.operation);
				}

				group.*memberPtr = value;
				} else
					throw std::runtime_error(" Compile rules only apply to scalar fields");
		}, field.ptr);
	}
}

bool meetsCondition(const entryDB& entries, EntryId id, const CompileCondition& condition) { // Expects compared fields to be scalar values
	const auto& entryField = entries.getField(id, entries.getFieldIndex(condition.field));

	return std::visit(
		[&](const auto& groupValue, const auto& entryValue) {
			using GroupValueType = std::remove_cvref_t<decltype(groupValue)>;
			using EntryValueType = std::remove_cvref_t<decltype(entryValue)>;

			if constexpr (!std::is_same_v<GroupValueType, EntryValueType>) {
				throw std::runtime_error(" Compared fields arent the same type. \n");
				return false;
			} else {

				switch (condition.type) {
					case CompileCondition::CONDITION_TYPE::EQUAL: {
						return entryValue == groupValue;
					}
					case CompileCondition::CONDITION_TYPE::NOT_EQUAL: {
						return entryValue != groupValue;
					}
					case CompileCondition::CONDITION_TYPE::GREATER_THAN: {
						return entryValue > groupValue;
					}
					case CompileCondition::CONDITION_TYPE::LESS_THAN: {
						return entryValue < groupValue;
					}
					case CompileCondition::CONDITION_TYPE::GREATER_EQUAL: {
						return entryValue >= groupValue;
					}
					case CompileCondition::CONDITION_TYPE::LESS_EQUAL: {
						return entryValue <= groupValue;
					}
					default:
						throw std::runtime_error("Invalid compile condition: " + std::to_string(static_cast<uint8_t>(condition.type)) + "\n");
						return false;
				}
			}
		}, condition.value, entryField);
}

} // namespace anime::domain
