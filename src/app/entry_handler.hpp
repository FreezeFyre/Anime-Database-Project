#pragma once

#include "net/transport/session.hpp"
#include "protocol/entry_messages.hpp"
#include "protocol/library_messages.hpp"

#include "../databases/library_service.hpp"

namespace anime::app { struct Context; }

namespace anime::app {

using anime::protocol::EntryGetRequest;
using anime::protocol::EntryUpdateRequest;
using anime::protocol::EntryCreateRequest;
using anime::protocol::EntryDeleteRequest;
using anime::protocol::EntrySearchRequest;
using anime::protocol::EntryFilterRequest;
using anime::protocol::EntryRequest;
using anime::protocol::EntryResponse;
using anime::protocol::LibraryResponse;
using anime::protocol::LibraryRequest;

class EntryRouter {
	public:
		explicit EntryRouter(anime::storage::LibraryService& service) : library(service) {}

		EntryResponse route(const EntryRequest& request, Context& context, anime::net::Session& session);

	private:
		anime::storage::LibraryService& library;
		EntryResponse handle(const EntryGetRequest& request, Context& context, anime::net::Session& session);
		EntryResponse handle(const EntryUpdateRequest& request, Context& context, anime::net::Session& session);
		EntryResponse handle(const EntryCreateRequest& request, Context& context, anime::net::Session& session);
		EntryResponse handle(const EntryDeleteRequest& request, Context& context, anime::net::Session& session);
		EntryResponse handle(const EntrySearchRequest& request, Context& context, anime::net::Session& session);
		EntryResponse handle(const EntryFilterRequest& request, Context& context, anime::net::Session& session);
};

class LibraryRouter {
	public:
		explicit LibraryRouter(anime::storage::LibraryService& service) : library(service) {}

		anime::protocol::LibraryResponse route(const anime::protocol::LibraryRequest& request, Context& context,
		                                       anime::net::Session& session);

	private:
		anime::storage::LibraryService& library;
		anime::protocol::LibraryResponse handle(const anime::protocol::CreateAnimeRequest& request, Context& context,
		                                        anime::net::Session& session);
		anime::protocol::LibraryResponse handle(const anime::protocol::UpdateAnimeRequest& request, Context& context,
		                                        anime::net::Session& session);
		anime::protocol::LibraryResponse handle(const anime::protocol::DeleteAnimeRequest& request, Context& context,
		                                        anime::net::Session& session);
		anime::protocol::LibraryResponse handle(const anime::protocol::AddEntriesRequest& request, Context& context,
		                                        anime::net::Session& session);
		anime::protocol::LibraryResponse handle(const anime::protocol::UpdateEntriesRequest& request, Context& context,
		                                        anime::net::Session& session);
		anime::protocol::LibraryResponse handle(const anime::protocol::TransferEntryRequest& request, Context& context,
		                                        anime::net::Session& session);
		anime::protocol::LibraryResponse handle(const anime::protocol::DeleteEntriesRequest& request, Context& context,
		                                        anime::net::Session& session);
		anime::protocol::LibraryResponse handle(const anime::protocol::GetAnimeRequest& request, Context& context,
		                                        anime::net::Session& session);
		anime::protocol::LibraryResponse handle(const anime::protocol::GetEntriesRequest& request, Context& context,
		                                        anime::net::Session& session);
		anime::protocol::LibraryResponse handle(const anime::protocol::SearchAnimeRequest& request, Context& context,
		                                        anime::net::Session& session);
};

} // namespace anime::app
