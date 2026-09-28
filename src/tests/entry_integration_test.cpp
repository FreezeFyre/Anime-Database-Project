#include "app/context.hpp"
#include "app/entry_handler.hpp"
#include "net/routing/router.hpp"
#include "net/protocol/messenger.hpp"
#include "net/https/httpCodec.hpp"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <iostream>

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

	anime::app::EntryRouter direct;
	const auto rejected=direct.route(EntryUpdateRequest{},context,session);
	assert(std::holds_alternative<EntryErrorResponse>(rejected.payload));
	assert(std::holds_alternative<EntryErrorResponse>(direct.route(EntryCreateRequest{{entries.front()}},context,session).payload));
	assert(std::holds_alternative<EntryErrorResponse>(direct.route(EntryDeleteRequest{{static_cast<anime::domain::EntryId>(entries.front().id)}},context,session).payload));
	const auto searched=direct.route(EntrySearchRequest{"Rascal"},context,session);
	assert(std::get<EntryIdsResponse>(searched.payload).ids==context.entries.searchEntryTitles("Rascal"));
	const auto filtered=direct.route(EntryFilterRequest{},context,session);
	assert(std::get<EntryIdsResponse>(filtered.payload).ids==context.entries.getFilteredIDS({}));
	session.role=anime::domain::USER_ACCOUNT_ROLE::EDITOR;
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
}

int main() {
	anime::app::Context context;
	fixtureRoundTrip(context);
	transportAndHandlerCompatibility(context);
	std::cout << "entry_integration_test passed\n";
}
