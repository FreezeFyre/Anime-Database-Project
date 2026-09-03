#include "app/entry_handler.hpp"
#include "app/context.hpp"
#include <mutex>
#include <shared_mutex>

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
	EntryGetResponse resp;
	try {
		{
			std::shared_lock lock(context.entriesMutex);
			resp.section = context.entries.getSection();
			resp.schema = context.entries.getSchema();
			resp.entries = context.entries.getEntries(request.ids);
		}
	} catch (const std::exception& e) {
		std::cerr << "Error generating payload: " << e.what() << "\n";
		return {EntryErrorResponse{e.what()}, false};
	}
	return {resp, false};
}

EntryResponse EntryRouter::handle(const EntryUpdateRequest& request, Context& context, Session& session) {
	try {
		{
			std::unique_lock lock(context.entriesMutex);
			context.entries.updateEntry(request.updates, session.role);
		}
	} catch (const std::exception& e) {
		std::cerr << "Error updating entries: " << e.what() << "\n";
		return {EntryErrorResponse{e.what()}, false};
	}

	return {EntrySuccessResponse(), false};
}
EntryResponse EntryRouter::handle(const EntryCreateRequest& request, Context& context, Session& session) {
	try {
		{
			std::unique_lock lock(context.entriesMutex);
			for (const Entry& entry : request.entries) {
				context.entries.addEntry(entry, session.role); // Sloppy role check? probably fine because of catch
			}
		}
	} catch (const std::exception& e) {
		std::cerr << "Error adding entry: " << e.what() << "\n";
		return {EntryErrorResponse{e.what()}, false};
	}

	return {EntrySuccessResponse(), false};
}
EntryResponse EntryRouter::handle(const EntryDeleteRequest& request, Context& context, Session& session) {
	try {
		// Temporarily lock deletion into a single id at a time
		if (request.ids.size() > 1) {throw std::runtime_error(" Can only delete one entry at a time right now. ");}
		{
			std::unique_lock lock(context.entriesMutex);
			context.entries.deleteEntry(request.ids[0], session.role);
		}
	} catch (const std::exception& e) {
		std::cerr << "Error deleting entry: " << e.what() << "\n";
		return {EntryErrorResponse{e.what()}, false};
	}

	return {EntrySuccessResponse(), false};
}
EntryResponse EntryRouter::handle(const EntrySearchRequest& request, Context& context, Session& session) {
	EntryIdsResponse resp;
	try {
		{
			std::shared_lock lock(context.entriesMutex);
			resp.ids = context.entries.searchEntryTitles(request.searchTerm);
		}
	} catch (const std::exception& e) {
		std::cerr << "Error searching entry titles: " << e.what() << "\n";
		return {EntryErrorResponse{e.what()}, false};
	}

	return {resp, false};
}
EntryResponse EntryRouter::handle(const EntryFilterRequest& request, Context& context, Session& session) {
	EntryIdsResponse resp;

	try {
		{
			std::shared_lock lock(context.entriesMutex);
			resp.ids = context.entries.getFilteredIDS(request.filters);
		}
	} catch (const std::exception& e) {
		std::cerr << "Error filtering entries: " << e.what() << "\n";
		return {EntryErrorResponse{e.what()}, false};
	}

	return {resp, false};
}

} // namespace anime::app
