#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>
#include <stdexcept>

#include "domain/types.hpp"
#include "schema/field.hpp"

namespace anime::domain {
	using anime::schema::FIELD_PRESENCE;
	using anime::schema::FieldInfo;
	using anime::schema::UPDATE_OPERATION;
	using anime::schema::VALUE_REFERENCE_TYPE;
	using anime::storage::recordField;
	using anime::storage::schemaField;
	using anime::domain::EntryId;
	using anime::domain::GroupId;
	using anime::domain::LookupId;

	struct Group {
		GroupId id = 0;
		std::uint32_t rank = 0;
		std::string name = "";
		std::string description = "";

		std::vector<EntryId> members = {};
		std::vector<LookupId> genres = {};
		std::uint64_t knownRuntimeMinutes = 0;
		std::uint32_t episodeCount = 0;
		std::uint32_t movieCount = 0;
		std::uint32_t seriesCount = 0;
		std::uint8_t fanServiceLevel = 0;
		std::uint8_t status = 0;
		bool runtimeComplete = false;

		std::uint64_t compiledFromEntryRevision = 0;
		std::uint32_t compilationVersion = 1;
	};

	using GroupMemberPtr = std::variant<
		std::uint8_t Group::*,
		std::uint16_t Group::*,
		std::uint32_t Group::*,
		std::uint64_t Group::*,
		std::string Group::*,
		bool Group::*,
		std::vector<std::uint8_t> Group::*,
		std::vector<std::uint32_t> Group::*
	>;

	enum class COMPILE_OPERATION {
		COUNT,
		SUM,
		UNION,
		CONCAT,
		MIN,
		MAX,
		AVG // not currently supported
	};

	struct CompileCondition {
		enum class CONDITION_TYPE { EQUAL, NOT_EQUAL, GREATER_THAN, LESS_THAN, GREATER_EQUAL, LESS_EQUAL };
		std::string field;
		recordField value;
		CONDITION_TYPE type;
	};


	struct CompilationRule {
		COMPILE_OPERATION operation;
		std::optional<std::string> source;
		std::optional<CompileCondition> condition = std::nullopt;
	};


	struct GroupField {
		FieldInfo info;
		GroupMemberPtr ptr;
		std::optional<CompilationRule> rule = std::nullopt;
	};


	inline const auto groupMetadata = std::to_array<GroupField>({
		GroupField{
			.info = {
				.name = "id",
				.type = schemaField::dataType::U32,
				.presence = FIELD_PRESENCE::REQUIRED
			},
			.ptr = &Group::id
		},GroupField{
			.info = {
				.name = "rank",
				.type = schemaField::dataType::U32,
				.mode = schemaField::filterMode::FM_RMV,
				.presence = FIELD_PRESENCE::REQUIRED
			},
			.ptr = &Group::rank
		},GroupField{
			.info = {
				.name = "name",
				.type = schemaField::dataType::STR
			},
			.ptr = &Group::name
		},GroupField{
			.info = {
				.name = "description",
				.type = schemaField::dataType::STR
			},
			.ptr = &Group::description
		},GroupField{
			.info = {
				.name = "members",
				.type = schemaField::dataType::VECU32,
				.presence = FIELD_PRESENCE::REQUIRED
			},
			.ptr = &Group::members
		},GroupField{
			.info = {
				.name = "genres",
				.type = schemaField::dataType::VECU8,
				.mode = schemaField::filterMode::FM_MULTI_VALUE
			},
			.ptr = &Group::genres,
			.rule = CompilationRule{
				.operation = COMPILE_OPERATION::UNION,
				.source = "genres"
			}
		},GroupField{
			.info = {
				.name = "knownRuntimeMinutes",
				.type = schemaField::dataType::U64
			},
			.ptr = &Group::knownRuntimeMinutes,
			.rule = CompilationRule{
				.operation = COMPILE_OPERATION::SUM,
				.source = "min_runtime"
			}
		},GroupField{
			.info = {
				.name = "episodeCount",
				.type = schemaField::dataType::U32,
				.presence = FIELD_PRESENCE::REQUIRED
			},
			.ptr = &Group::episodeCount,
			.rule = CompilationRule{
				.operation = COMPILE_OPERATION::SUM,
				.source = "episode_count",
				.condition = CompileCondition{.field = "format", .value = std::uint32_t{0}, .type = CompileCondition::CONDITION_TYPE::EQUAL}
			}
		},GroupField{
			.info = {
				.name = "movieCount",
				.type = schemaField::dataType::U32,
				.presence = FIELD_PRESENCE::REQUIRED
			},
			.ptr = &Group::movieCount,
			.rule = CompilationRule{
				.operation = COMPILE_OPERATION::COUNT,
				.condition = CompileCondition{.field = "format", .value = std::uint32_t{1}, .type = CompileCondition::CONDITION_TYPE::EQUAL}
			}
		},GroupField{
			.info = {
				.name = "seriesCount",
				.type = schemaField::dataType::U32,
				.presence = FIELD_PRESENCE::REQUIRED
			},
			.ptr = &Group::seriesCount,
			.rule = CompilationRule{
				.operation = COMPILE_OPERATION::COUNT,
				.condition = CompileCondition{.field = "format", .value = std::uint32_t{0}, .type = CompileCondition::CONDITION_TYPE::EQUAL}
			}
		},GroupField{
			.info = {
				.name = "fanServiceLevel",
				.type = schemaField::dataType::U8
			},
			.ptr = &Group::fanServiceLevel,
			.rule = CompilationRule{
				.operation = COMPILE_OPERATION::AVG,
				.source = "fan_service"
			}
		},GroupField{
			.info = {
				.name = "status",
				.type = schemaField::dataType::U8
			},
			.ptr = &Group::status,
			.rule = CompilationRule{
				.operation = COMPILE_OPERATION::MAX,
				.source = "status"
			}
		},GroupField{
			.info = {
				.name = "runtimeComplete",
				.type = schemaField::dataType::U64
			},
			.ptr = &Group::runtimeComplete,
			.rule = CompilationRule{
				.operation = COMPILE_OPERATION::COUNT
			}
		},GroupField{
			.info = {
				.name = "compiledFromEntryRevision",
				.type = schemaField::dataType::U64
			},
			.ptr = &Group::compiledFromEntryRevision,
			.rule = CompilationRule{
				.operation = COMPILE_OPERATION::COUNT
			}
		},GroupField{
			.info = {
				.name = "compilationVersion",
				.type = schemaField::dataType::U64
			},
			.ptr = &Group::compilationVersion,
			.rule = CompilationRule{
				.operation = COMPILE_OPERATION::COUNT
			}
		},
	});

	inline const std::unordered_map<std::string, size_t> groupFieldNames = [] {
		std::unordered_map<std::string, size_t> result;
		for (size_t i = 0; i < groupMetadata.size(); ++i)
			result.emplace(groupMetadata[i].info.name, i);
		return result;
	}();

	struct GroupUpdate {
		GroupId id;
		UPDATE_OPERATION op;
		size_t fieldIndex;
		recordField value;
	};

} // namespace anime::domain
