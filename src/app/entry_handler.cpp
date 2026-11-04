#include "app/entry_handler.hpp"
#include "app/context.hpp"
#include <mutex>
#include <shared_mutex>

#include "../databases/library_service.hpp"

namespace anime::app {

using namespace anime::domain;
using namespace anime::protocol;
using namespace anime::schema;
using namespace anime::storage;
using anime::net::Session;

EntryResponse EntryRouter::route(const EntryRequest& request, Context& context, Session& session) {
	return std::visit([&](const auto& payload) {
		return handle(payload, context, session);
	}, request);
}

EntryResponse EntryRouter::handle(const EntryGetRequest& request, Context& context, Session& session) {
	return library.getEntries(request, session);
}

EntryResponse EntryRouter::handle(const EntryUpdateRequest& request, Context& context, Session& session) {
	return library.updateEntries(request, session);
}
EntryResponse EntryRouter::handle(const EntryCreateRequest& request, Context& context, Session& session) {
	return library.createEntries(request, session);
}
EntryResponse EntryRouter::handle(const EntryDeleteRequest& request, Context& context, Session& session) {
	return library.deleteEntries(request, session);
}
EntryResponse EntryRouter::handle(const EntrySearchRequest& request, Context& context, Session& session) {
	return library.searchEntries(request, session);
}
EntryResponse EntryRouter::handle(const EntryFilterRequest& request, Context& context, Session& session) {
	return library.filterEntries(request, session);
}


LibraryResponse LibraryRouter::route(const LibraryRequest& request, Context& context, anime::net::Session& session) {
	return std::visit([&](const auto& payload) {
		return handle(payload, context, session);
	}, request);
}

LibraryResponse LibraryRouter::handle(const CreateAnimeRequest& request, Context& context, anime::net::Session& session) {
	return library.createAnime(request, session);
}
LibraryResponse LibraryRouter::handle(const UpdateAnimeRequest& request, Context& context, anime::net::Session& session) {
	return library.updateAnime(request,session);
}
LibraryResponse LibraryRouter::handle(const DeleteAnimeRequest& request, Context& context, anime::net::Session& session) {
	return library.deleteAnime(request,session);
}
LibraryResponse LibraryRouter::handle(const AddEntriesRequest& request, Context& context, anime::net::Session& session) {
	return library.addEntries(request,session);
}
LibraryResponse LibraryRouter::handle(const UpdateEntriesRequest& request, Context& context, anime::net::Session& session) {
	return library.updateEntries(request,session);
}
LibraryResponse LibraryRouter::handle(const TransferEntryRequest& request, Context& context, anime::net::Session& session) {
	return library.transferEntry(request,session);
}
LibraryResponse LibraryRouter::handle(const DeleteEntriesRequest& request, Context& context, anime::net::Session& session) {
	return library.deleteEntries(request,session);
}
LibraryResponse LibraryRouter::handle(const GetAnimeRequest& request, Context& context, anime::net::Session& session) {
	return library.getAnime(request,session);
}
LibraryResponse LibraryRouter::handle(const GetEntriesRequest& request, Context& context, anime::net::Session& session) {
	return library.getEntries(request,session);
}
LibraryResponse LibraryRouter::handle(const SearchAnimeRequest& request, Context& context, anime::net::Session& session) {
	return library.searchAnime(request,session);
}

} // namespace anime::app
