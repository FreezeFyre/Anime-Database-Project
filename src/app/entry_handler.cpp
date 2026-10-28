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

} // namespace anime::app
