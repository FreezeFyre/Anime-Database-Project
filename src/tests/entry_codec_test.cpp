#include "protocol/entry_codec.hpp"
#include "domain/entry_schema.hpp"

#include <cassert>
#include <functional>
#include <iostream>
#include <limits>

namespace {
using namespace anime::protocol;
using namespace anime::domain;
using namespace anime::storage;
using anime::schema::UPDATE_OPERATION;

void expectError(const std::function<void()>& action, const std::string& message) {
	try { action(); } catch (const std::runtime_error& error) {
		assert(error.what() == message);
		return;
	}
	throw std::runtime_error("Expected error: " + message);
}

void requestFixtures() {
	EntryWebSocketCodec codec;
	const ByteVector ids{0,0,0,2, 0,0,0,1, 0,0,1,0};
	assert((std::get<EntryGetRequest>(codec.decode(ACTION_TYPE::ACT_GET, ids)).ids == std::vector<EntryId>{1,256}));
	assert((std::get<EntryDeleteRequest>(codec.decode(ACTION_TYPE::ACT_DELETE, ids)).ids == std::vector<EntryId>{1,256}));
	const ByteVector search{0,0,0,3,'a','b','c'};
	assert(std::get<EntrySearchRequest>(codec.decode(ACTION_TYPE::ACT_SEARCH, search)).searchTerm == "abc");
	const ByteVector emptyFilters{0,0,0,0};
	assert(std::get<EntryFilterRequest>(codec.decode(ACTION_TYPE::ACT_FILTER, emptyFilters)).filters.empty());
	ByteWriter filters;
	filters.U32(1); filters.STR("rank"); filters.U8(31);
	filters.VECU32({1}); filters.VECU32({2,3}); filters.VECU32({4});
	filters.U32(5); filters.U32(10);
	const auto filter=std::get<EntryFilterRequest>(codec.decode(ACTION_TYPE::ACT_FILTER,filters.buffer)).filters.at(0);
	assert(filter.name=="rank" && filter.includeAll==std::vector<std::uint32_t>{1});
	assert((filter.includeAny==std::vector<std::uint32_t>{2,3}));
	assert(filter.exclude==std::vector<std::uint32_t>{4});
	assert(filter.range.min==5 && filter.range.max==10);

	ByteWriter updates;
	updates.U32(6);
	for (std::uint8_t type=0; type<6; ++type) {
		updates.U32(42);
		updates.U8(static_cast<std::uint8_t>(UPDATE_OPERATION::SET_FIELD));
		updates.U8(type);
		updates.U8(type);
		switch(static_cast<dataType>(type)) {
		case dataType::U8: updates.U8(255); break;
		case dataType::U16: updates.U16(4096); break;
		case dataType::U32: updates.U32(100000); break;
		case dataType::STR: updates.STR("updated"); break;
		case dataType::VECU8: updates.VECU8({1,255}); break;
		case dataType::VECU16: updates.VECU16({1,4096}); break;
		default: break;
		}
	}
	const auto decoded=std::get<EntryUpdateRequest>(codec.decode(ACTION_TYPE::ACT_UPDATE, updates.buffer));
	assert(decoded.updates.size()==6);
	assert(decoded.updates[0].id==42);
	assert(std::get<int>(decoded.updates[0].value)==255);
	assert(std::get<int>(decoded.updates[1].value)==4096);
	assert(std::get<int>(decoded.updates[2].value)==100000);
	assert(std::get<std::string>(decoded.updates[3].value)=="updated");
	assert((std::get<std::vector<int>>(decoded.updates[4].value)==std::vector<int>{1,255}));
	assert((std::get<std::vector<int>>(decoded.updates[5].value)==std::vector<int>{1,4096}));
	expectError([&]{codec.decode(ACTION_TYPE::ACT_UNKNOWN, {});}, "Unknown entry action type.");
}

void responseFixturesAndCreateRoundTrip() {
	EntryWebSocketCodec codec;
	assert((codec.encode({EntryIdsResponse{{1,256}},false}) == ByteVector{0,0,0,2,0,0,0,1,0,0,1,0}));
	assert((codec.encode({EntrySuccessResponse{"OK"},false}) == ByteVector{0,0,0,2,'O','K'}));
	assert((codec.encode({EntryErrorResponse{"bad"},false}) == ByteVector{0,0,0,3,'b','a','d'}));

	Entry original;
	original.id=42; original.rank=7; original.title="Round trip"; original.alt_title="Alternative";
	original.format=2; original.status=1; original.genres={1,2}; original.channels={3};
	original.audios={4}; original.subtitles={5}; original.fan_service=6;
	original.episode_count=12; original.min_runtime=1440; original.characters={500}; original.ip_group=9;
	EntryGetResponse response;
	for(const auto& field:entryMetadata) response.schema.push_back({field.info.name,field.info.type,field.info.mode});
	response.entries.push_back(original);
	const auto bytes=codec.encode({response,false});
	const auto created=std::get<EntryCreateRequest>(codec.decode(ACTION_TYPE::ACT_CREATE,bytes));
	assert(created.entries.size()==1);
	for(const auto& field:entryMetadata) {
		std::visit([&](auto member){assert(original.*member==created.entries[0].*member);},field.ptr);
	}
	response.entries=created.entries;
	assert(codec.encode({response,false})==bytes);
}

void schemaCompatibility() {
	schemaData required{
		{"title",schemaField::dataType::STR,schemaField::filterMode::FM_NONE},
		{"id",schemaField::dataType::U32,schemaField::filterMode::FM_NONE},
		{"rank",schemaField::dataType::U32,schemaField::filterMode::FM_RMV}
	};
	assert((validateEntrySchema(required)==std::vector<size_t>{2,0,1}));
	auto duplicate=required; duplicate.push_back(required[0]);
	expectError([&]{validateEntrySchema(duplicate);},"Duplicate schema field: title");
	auto wrong=required; wrong[0].type=schemaField::dataType::U8;
	expectError([&]{validateEntrySchema(wrong);},"Schema field has incorrect data type: title");
	auto unknown=required; unknown[0].name="missing";
	expectError([&]{validateEntrySchema(unknown);},"Schema field not found in EntryMeta: missing");
	required.erase(required.begin()+1);
	expectError([&]{validateEntrySchema(required);},"Required field is missing from schema: id");
}
}

int main() {
	requestFixtures();
	responseFixturesAndCreateRoundTrip();
	schemaCompatibility();
	std::cout << "entry_codec_test passed\n";
}
