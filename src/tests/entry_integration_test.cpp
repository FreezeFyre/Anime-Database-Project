#include "app/context.hpp"
#include "app/entry_handler.hpp"
#include "net/routing/router.hpp"
#include "net/protocol/messenger.hpp"
#include "net/https/httpCodec.hpp"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>

namespace {
using namespace anime::protocol;
namespace fs=std::filesystem;

struct TemporaryDirectory {
	fs::path path=fs::temp_directory_path() / ("anime-refactor-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
	TemporaryDirectory(){fs::create_directory(path);}
	~TemporaryDirectory(){std::error_code error;fs::remove_all(path,error);}
};

void fixtureRoundTrip(anime::app::Context& context) {
	const fs::path fixtures=ANIME_FIXTURE_DIR;
	context.entries.loadEntries((fixtures/"animeDatabase.1.dat").string());
	context.entries.loadLookupTables((fixtures/"animeLookupTables.1.dat").string());
	context.users.load((fixtures/"userDatabase.1.dat").string());
	TemporaryDirectory temp;
	context.entries.saveEntries((temp.path/"entries.dat").string());
	context.users.save((temp.path/"users.dat").string());
	anime::storage::entryDB reloaded;
	reloaded.loadEntries((temp.path/"entries.dat").string());
	const auto before=context.entries.getEntries({0});
	const auto after=reloaded.getEntries({0});
	assert(!before.empty() && before.size()==after.size());
	for(size_t i=0;i<before.size();++i) {
		for(const auto& field:anime::domain::entryMetadata) {
			std::visit([&](auto member){assert(before[i].*member==after[i].*member);},field.ptr);
		}
	}
	anime::storage::userDB users;
	users.load((temp.path/"users.dat").string());
	const auto kind=anime::storage::fileType::HEADER_DATABASE;
	const auto header=context.users.getHeader(kind);
	assert(header.sectionCount==users.getHeader(kind).sectionCount);
	for(size_t section=0;section<header.sectionCount;++section) {
		const auto count=context.users.getSection(kind,section).recordCount;
		assert(count==users.getSection(kind,section).recordCount);
		for(size_t row=0;row<count;++row) assert(context.users.getRecord(kind,section,row)==users.getRecord(kind,section,row));
	}
}

void transportAndHandlerCompatibility(anime::app::Context& context) {
	ByteWriter payload; payload.write(std::vector<std::uint32_t>{0});
	ByteWriter envelope;
	envelope.write(static_cast<std::uint32_t>(ENCODE_VERSION));
	envelope.write(std::uint8_t{1}); envelope.write(std::uint8_t{1}); envelope.write(std::uint8_t{1});
	envelope.write(std::uint32_t{1234});
	envelope.write(anime::storage::helpers::checkedConvert<std::uint32_t>(payload.buffer.size()));
	envelope.buffer.insert(envelope.buffer.end(),payload.buffer.begin(),payload.buffer.end());
	WebSocketCodec codec;
	auto request=codec.decode(envelope.buffer);
	anime::net::Session session;
	anime::app::Router router;
	const auto response=router.route(request,context,session);
	assert(response.id==1234 && !response.closeConnection);
	const auto& entries=std::get<EntryGetResponse>(std::get<EntryResponse>(response.payload).payload).entries;
	assert(entries.size()==context.entries.getSize());
	const auto wire=codec.encode(request,response);
	ByteReader reader{wire};
	assert(reader.read<std::uint32_t>()==ENCODE_VERSION); assert(reader.read<std::uint8_t>()==2);
	assert(reader.read<std::uint8_t>()==1); assert(reader.read<std::uint8_t>()==1); assert(reader.read<std::uint32_t>()==1234);
	assert(reader.read<std::uint32_t>()==wire.size()-15);

	anime::storage::LibraryService library{context.entries,context.groups,context.entriesMutex,context.groupsMutex};
	anime::app::EntryRouter direct{library};
	const auto rejected=direct.route(EntryUpdateRequest{},context,session);
	assert(std::holds_alternative<EntryErrorResponse>(rejected.payload));
	assert(std::holds_alternative<EntryErrorResponse>(direct.route(EntryCreateRequest{{entries.front()}},context,session).payload));
	assert(std::holds_alternative<EntryErrorResponse>(direct.route(EntryDeleteRequest{{static_cast<anime::domain::EntryId>(entries.front().id)}},context,session).payload));
	const auto searched=direct.route(EntrySearchRequest{"Rascal"},context,session);
	assert(std::get<EntryIdsResponse>(searched.payload).ids==context.entries.searchEntryTitles("Rascal"));
	const auto filtered=direct.route(EntryFilterRequest{},context,session);
	assert(std::get<EntryIdsResponse>(filtered.payload).ids==context.entries.getFilteredIDS({}));
	session.role=anime::domain::USER_ACCOUNT_ROLE::EDITOR;
	const auto emptyDelete=direct.route(EntryDeleteRequest{},context,session);
	assert(std::holds_alternative<EntryErrorResponse>(emptyDelete.payload));
	assert(std::get<EntryErrorResponse>(emptyDelete.payload).message.find("At least one entry id")!=std::string::npos);
	assert(context.entries.getSize()==entries.size());
	const auto accepted=direct.route(EntryUpdateRequest{},context,session);
	assert(std::holds_alternative<EntrySuccessResponse>(accepted.payload));

	// Preserve the existing transport boundary: entries are not an HTTP API.
	anime::net::http::HttpAppCodec http;
	anime::net::http::HttpRequest httpRequest{"GET","/entry/get","HTTP/1.1",{}, {}};
	bool unsupported=false;
	try {http.decode(httpRequest);} catch(const std::runtime_error& error) {
		unsupported=std::string(error.what())=="Unkown/Unsupported domain in target: /entry/get";
	}
	assert(unsupported);
	auto copy=response;
	const auto httpResponse=http.encode(copy);
	assert(httpResponse.statusCode==405);
}

void unfinishedLibraryOperationsDoNotMutate() {
	anime::app::Context context;
	const fs::path fixtures=ANIME_FIXTURE_DIR;
	context.entries.loadEntries(fixtures/"animeDatabase.crunchyroll.dat");
	context.groups.load(fixtures/"groupDatabase.crunchyroll.dat");
	anime::storage::LibraryService library{context.entries,context.groups,context.entriesMutex,context.groupsMutex};
	anime::net::Session session;
	session.signedIn=true;
	session.role=anime::domain::USER_ACCOUNT_ROLE::ADMIN;

	const auto snapshot=[&] {
		std::vector<anime::storage::recordData> entries, groups;
		const auto kind=anime::storage::fileType::HEADER_DATABASE;
		for(size_t i=0;i<context.entries.getSize();++i) entries.push_back(context.entries.getRecord(kind,0,i));
		for(size_t i=0;i<context.groups.getSection().recordCount;++i) groups.push_back(context.groups.getRecord(kind,0,i));
		return std::make_pair(entries,groups);
	};
	const auto before=snapshot();
	assert(!before.first.empty() && !before.second.empty());
	const auto entry=context.entries.getEntries({0}).front();
	const auto groupId=std::get<anime::domain::GroupId>(before.second.front().front());
	const auto check=[&](const auto& result, std::string_view operation) {
		assert(!result.has_value());
		assert(result.error().code==LIBRARY_ERROR_CODE::NOT_IMPLEMENTED);
		assert(result.error().err==std::string(operation)+" is not implemented.");
		assert(snapshot()==before);
	};
	const Update groupUpdate{groupId,anime::schema::UPDATE_OPERATION::SET_FIELD,2,anime::storage::dataType::STR,std::string{"Draft title"}};
	const Update entryUpdate{entry.id,anime::schema::UPDATE_OPERATION::SET_FIELD,2,anime::storage::dataType::STR,std::string{"Draft title"}};
	anime::domain::Group newGroup;
	newGroup.id=10000;
	newGroup.name="Unimplemented group";
	check(library.createAnime(CreateAnimeRequest{newGroup,{entry}},session),"createAnime");
	check(library.updateAnime(UpdateAnimeRequest{{groupUpdate}},session),"updateAnime");
	check(library.deleteAnime(DeleteAnimeRequest{groupId},session),"deleteAnime");
	check(library.addEntries(AddEntriesRequest{groupId,{entry}},session),"addEntries");
	check(library.updateEntries(UpdateEntriesRequest{{entryUpdate}},session),"updateEntries");
	check(library.transferEntry(TransferEntryRequest{entry.id,groupId},session),"transferEntry");
	check(library.deleteEntries(DeleteEntriesRequest{{entry.id}},session),"deleteEntries");
	check(library.getAnime(GetAnimeRequest{{groupId}},session),"getAnime");
	check(library.getEntries(GetEntriesRequest{{entry.id}},session),"getEntries");
	check(library.searchAnime(SearchAnimeRequest{"Rascal"},session),"searchAnime");
	check(library.filterAnime(FilterAnimeRequest{},session),"filterAnime");
}
}

int main(int argc, char* argv[]) {
	anime::app::Context context;
	if(argc==2 && std::string_view(argv[1])=="--library-service") {
		const fs::path fixtures=ANIME_FIXTURE_DIR;
		context.entries.loadEntries(fixtures/"animeDatabase.crunchyroll.dat");
		context.entries.loadLookupTables(fixtures/"animeLookupTables.1.dat");
		transportAndHandlerCompatibility(context);
		unfinishedLibraryOperationsDoNotMutate();
		std::cout << "library_service_test passed\n";
		return 0;
	}
	fixtureRoundTrip(context);
	transportAndHandlerCompatibility(context);
	std::cout << "entry_integration_test passed\n";
}
