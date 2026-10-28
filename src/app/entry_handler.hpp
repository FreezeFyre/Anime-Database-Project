#pragma once

#include "protocol/entry_messages.hpp"
#include "net/transport/session.hpp"

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

} // namespace anime::app
