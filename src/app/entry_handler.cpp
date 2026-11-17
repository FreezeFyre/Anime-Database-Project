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

	EntryResponse EntryRouter::route(const EntryRequest& request, Context& context, const std::string& token) {
		Session session;
		if (!token.empty()) {
			const auto stored = context.sessions.resolve(token);
			if (!stored || !stored->userId) {
				return {EntryErrorResponse{"Invalid or expired session."}, false};
			}
			try {
				std::shared_lock lock(context.usersMutex);
				const auto& user = context.users.getUser(*stored->userId);
				if (user.disabled) {
					return {EntryErrorResponse{"User account is disabled."}, false};
				}
				session.signedIn = true;
				session.userId = user.id;
				session.role = user.role;
			} catch (const std::out_of_range&) {
				return {EntryErrorResponse{"Session user does not exist."}, false};
			} catch (const std::runtime_error&) {
				return {EntryErrorResponse{"Session user does not exist."}, false};
			}
		}
		const bool mutation = std::holds_alternative<EntryCreateRequest>(request) ||
		                      std::holds_alternative<EntryUpdateRequest>(request) ||
		                      std::holds_alternative<EntryDeleteRequest>(request);
		if (mutation && (!session.signedIn || session.role < USER_ACCOUNT_ROLE::EDITOR)) {
			return {EntryErrorResponse{"Editor permission is required."}, false};
		}
		return std::visit([&](const auto& payload) { return handle(payload, context, session); }, request);
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

	LibraryResponse LibraryRouter::route(const LibraryRequest& request, Context& context, const std::string& token) {
		Session session;
		if (!token.empty()) {
			const auto stored = context.sessions.resolve(token);
			if (!stored || !stored->userId) {
				return ErrorResponse{LIBRARY_ERROR_CODE::INVALID_USER_PERMISSION_LEVEL, "Invalid or expired session."};
			}
			try {
				std::shared_lock lock(context.usersMutex);
				const auto& user = context.users.getUser(*stored->userId);
				if (user.disabled) {
					return ErrorResponse{LIBRARY_ERROR_CODE::INVALID_USER_PERMISSION_LEVEL,
					                     "User account is disabled."};
				}
				session.signedIn = true;
				session.userId = user.id;
				session.role = user.role;
			} catch (const std::out_of_range&) {
				return ErrorResponse{LIBRARY_ERROR_CODE::INVALID_USER_PERMISSION_LEVEL, "Session user does not exist."};
			} catch (const std::runtime_error&) {
				return ErrorResponse{LIBRARY_ERROR_CODE::INVALID_USER_PERMISSION_LEVEL, "Session user does not exist."};
			}
		}
		const bool read = std::holds_alternative<GetAnimeRequest>(request) ||
		                  std::holds_alternative<GetEntriesRequest>(request) ||
		                  std::holds_alternative<SearchAnimeRequest>(request);
		if (!read && (!session.signedIn || session.role < USER_ACCOUNT_ROLE::EDITOR)) {
			return ErrorResponse{LIBRARY_ERROR_CODE::INVALID_USER_PERMISSION_LEVEL, "Editor permission is required."};
		}
		return std::visit([&](const auto& payload) { return handle(payload, context, session); }, request);
	}

	LibraryResponse LibraryRouter::handle(const CreateAnimeRequest& request, Context& context,
	                                      anime::net::Session& session) {
		return library.createAnime(request, session);
	}
	LibraryResponse LibraryRouter::handle(const UpdateAnimeRequest& request, Context& context,
	                                      anime::net::Session& session) {
		return library.updateAnime(request, session);
	}
	LibraryResponse LibraryRouter::handle(const DeleteAnimeRequest& request, Context& context,
	                                      anime::net::Session& session) {
		return library.deleteAnime(request, session);
	}
	LibraryResponse LibraryRouter::handle(const AddEntriesRequest& request, Context& context,
	                                      anime::net::Session& session) {
		return library.addEntries(request, session);
	}
	LibraryResponse LibraryRouter::handle(const UpdateEntriesRequest& request, Context& context,
	                                      anime::net::Session& session) {
		return library.updateEntries(request, session);
	}
	LibraryResponse LibraryRouter::handle(const TransferEntryRequest& request, Context& context,
	                                      anime::net::Session& session) {
		return library.transferEntry(request, session);
	}
	LibraryResponse LibraryRouter::handle(const DeleteEntriesRequest& request, Context& context,
	                                      anime::net::Session& session) {
		return library.deleteEntries(request, session);
	}
	LibraryResponse LibraryRouter::handle(const GetAnimeRequest& request, Context& context,
	                                      anime::net::Session& session) {
		return library.getAnime(request, session);
	}
	LibraryResponse LibraryRouter::handle(const GetEntriesRequest& request, Context& context,
	                                      anime::net::Session& session) {
		return library.getEntries(request, session);
	}
	LibraryResponse LibraryRouter::handle(const SearchAnimeRequest& request, Context& context,
	                                      anime::net::Session& session) {
		return library.searchAnime(request, session);
	}

} // namespace anime::app
