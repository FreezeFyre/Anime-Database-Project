#include "../net/routing/router.hpp"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <iostream>
#include <vector>

namespace {

constexpr EntryId frierenId = 50;
constexpr EntryId kaguyaId = 51;

Entry makeEntry(const std::string& title, const std::string& altTitle) {
	Entry entry;
	entry.rank = 0;
	entry.title = title;
	entry.alt_title = altTitle;
	entry.format = 0;
	entry.status = 0;
	entry.genres = { 0 };
	entry.channels = { 0 };
	entry.audios = { 0 };
	entry.subtitles = { 0 };
	entry.fan_service = 1;
	entry.episode_count = 12;
	entry.min_runtime = 24;
	entry.characters = { 10 };
	entry.ip_group = 0;
	return entry;
}

void seedEntries(entryDB& db) {
	const std::filesystem::path lookupPath =
		std::filesystem::path(__FILE__).parent_path().parent_path() / "io" / "animeLookupTables.1.dat";

	db.loadLookupTables(lookupPath.string());
	db.addEntry(makeEntry("Frieren: Beyond Journey's End", "Sousou no Frieren"), UserRole::ADMIN);
	db.addEntry(makeEntry("Kaguya-sama: Love is War", "Kaguya-sama wa Kokurasetai"), UserRole::ADMIN);
}

bool contains(const std::vector<EntryId>& ids, EntryId id) {
	return std::find(ids.begin(), ids.end(), id) != ids.end();
}

void searchesTitleAndAltTitle() {
	entryDB db;
	seedEntries(db);

	std::vector<EntryId> titleMatch = db.searchEntryTitles("frieren");
	assert(titleMatch.size() == 1);
	assert(titleMatch[0] == frierenId);

	std::vector<EntryId> altTitleMatch = db.searchEntryTitles("kokurasetai");
	assert(altTitleMatch.size() == 1);
	assert(altTitleMatch[0] == kaguyaId);

	std::vector<EntryId> caseInsensitiveMatch = db.searchEntryTitles("LOVE IS WAR");
	assert(caseInsensitiveMatch.size() == 1);
	assert(caseInsensitiveMatch[0] == kaguyaId);

	assert(db.searchEntryTitles("not in database").empty());
	assert(db.searchEntryTitles("").empty());
}

void searchRouteReturnsEncodedIds() {
	Context context;
	seedEntries(context.entries);

	ByteWriter writer;
	writer.STR("journey");

	MessageCodec codec;
	Message request = codec.construct(
		COM_TYPE::REQ,
		DOMAIN_TYPE::ENTRY,
		ACTION_TYPE::ACT_SEARCH,
		123,
		static_cast<uint32_t>(writer.buffer.size()),
		writer.buffer
	);

	entryRouter router;
	Session session;
	RouteResult routeResult = router.route(request, context, session);

	assert(routeResult.message.comType == COM_TYPE::RESP);
	assert(routeResult.message.domain == DOMAIN_TYPE::ENTRY);
	assert(routeResult.message.action == ACTION_TYPE::ACT_SEARCH);
	assert(routeResult.message.id == 123);
	assert(routeResult.message.payloadSize == routeResult.message.payload.size());

	ByteReader reader{ routeResult.message.payload };
	std::vector<EntryId> ids = reader.VECU32();
	assert(ids.size() == 1);
	assert(ids[0] == frierenId);
}

}

int main() {
	searchesTitleAndAltTitle();
	searchRouteReturnsEncodedIds();

	std::cout << "entry_search_test passed\n";
	return 0;
}
