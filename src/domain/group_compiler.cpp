#include "group_compiler.hpp"
#include "group.hpp"
#include "../databases/entry_database.hpp"

#include <unordered_set>

namespace anime::domain::group {

void compileGroup(Group& group, const entryDB& entries) {
	for(const auto& field : groupMetadata) {
		if(!field.rule){continue;}
		const auto& rule = *field.rule;

		std::visit([&](const auto& memberPtr){
			auto data = prepField(group, memberPtr, rule.operation); // maybe use auto data = std::invoke_result_t<decltype(memberPtr), group>{};

			for(const EntryId id : group.members) {
				if(!entries.contains(id)) {
					throw std::runtime_error(" Group member id: " + std::to_string(id) + ", DNE in entries. ");
				}
				if(rule.condition && !meetsCondition(entries, id, *rule.condition)){
					continue;
				}
				if (COMPILE_OPERATION::COUNT == rule.operation) {
					if constexpr (UnsignedInteger<std::remove_cvref_t<decltype(data)>>) {
						data++;
					} else 
						throw std::runtime_error(" Count operation expects a unsigned integer field. ");
				} else {
					std::visit([&](const auto& entryFieldValue) {
						group::accumulate(data, entryFieldValue, rule.operation);
					}, entries.getField(id, entries.getFieldIndex(*rule.source)));
				}
			}
			group.*memberPtr = data;
		}, field.ptr);
	}
}



bool meetsCondition(const entryDB& entries, EntryId id, const CompileCondition& condition) {
	const auto& entryField = entries.getField(id, entries.getFieldIndex(condition.field));

	return std::visit(
		[&](const auto& groupValue, const auto& entryValue) {
			using ConditionType = std::remove_cvref_t<decltype(groupValue)>;
			using EntryType = std::remove_cvref_t<decltype(entryValue)>;

			switch (condition.type) {
				case CompileCondition::CONDITION_TYPE::EQUAL: { //if types match or both are scalar
					if constexpr (std::is_same_v<ConditionType,EntryType> || BothIntegral<ConditionType,EntryType>) {
						return entryValue == groupValue;
					} else
						throw std::runtime_error("");
				}
				case CompileCondition::CONDITION_TYPE::NOT_EQUAL: { // if types match or both are scalar, if types dont match throw and return false
					if constexpr (std::is_same_v<ConditionType,EntryType> || BothIntegral<ConditionType,EntryType>) {
						return entryValue != groupValue;
					} else
						throw std::runtime_error("");
				}
				case CompileCondition::CONDITION_TYPE::GREATER_THAN: { // Scalar only
					if constexpr (BothIntegral<ConditionType,EntryType>) {
						return entryValue > groupValue;
					} else
						throw std::runtime_error("");
				}
				case CompileCondition::CONDITION_TYPE::LESS_THAN: { // Scalar only
					if constexpr (BothIntegral<ConditionType,EntryType>) {
						return entryValue < groupValue;
					} else
						throw std::runtime_error("");
				}
				case CompileCondition::CONDITION_TYPE::GREATER_EQUAL: { // Scalar only
					if constexpr (BothIntegral<ConditionType,EntryType>) {
						return entryValue >= groupValue;
					} else
						throw std::runtime_error("");
				}
				case CompileCondition::CONDITION_TYPE::LESS_EQUAL: { // Scalar only
					if constexpr (BothIntegral<ConditionType,EntryType>) {
						return entryValue <= groupValue;
					} else
						throw std::runtime_error("");
				}
				case CompileCondition::CONDITION_TYPE::CONTAINS: { // Vector or string
					if constexpr (is_vector_v<EntryType> && std::integral<ConditionType>) {
						return std::ranges::contains(entryValue, groupValue);
					} else if constexpr (std::same_as<EntryType, std::string> && std::same_as<ConditionType, std::string>) {
						return entryValue.contains(groupValue);
					} else
						throw std::runtime_error("");
				}
				default:
					throw std::runtime_error("Invalid compile condition: " + std::to_string(static_cast<uint8_t>(condition.type)) + "\n");
					return false;
			}
		
		}, condition.value, entryField);
}

} // namespace anime::domain
