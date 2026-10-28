#include "library_service.hpp"

#include "../protocol/library_messages.hpp"
#include "../net/transport/session.hpp"
#include "../schema/field.hpp"

#include <mutex>
#include <shared_mutex>
#include <ranges>

namespace anime::storage {

using namespace anime::protocol;
using anime::net::Session;
using anime::schema::UPDATE_OPERATION;

LibraryService::LibraryService(entryDB& e, groupDB& g, std::shared_mutex& eMutex, std::shared_mutex& gMutex)
	: entries(e), groups(g), entriesMutex(eMutex), groupsMutex(gMutex) {};

EntryResponse LibraryService::getEntries(const EntryGetRequest& request, const Session& session) {
	try {
		{
			std::shared_lock lock(entriesMutex);
			return {EntryGetResponse{
				.section = entries.getSection(),
				.schema = entries.getSchema(),
				.entries = entries.getEntries(request.ids)
			}, false};
		}
	} catch (const std::exception& e) {
		std::cerr << "Error generating payload: " << e.what() << "\n";
		return {EntryErrorResponse{e.what()}, false};
	}
}
EntryResponse LibraryService::updateEntries(const EntryUpdateRequest& request, const Session& session) {
	try {
		{
			std::unique_lock lock(entriesMutex);
			entries.updateEntry(request.updates, session.role);
			return {EntrySuccessResponse(), false};
		}
	} catch (const std::exception& e) {
		std::cerr << "Error updating entries: " << e.what() << "\n";
		return {EntryErrorResponse{e.what()}, false};
	}
}
EntryResponse LibraryService::createEntries(const EntryCreateRequest& request, const Session& session) {
	try {
		{
			std::unique_lock lock(entriesMutex);
			for (const Entry& entry : request.entries) { // need to check group existence
				entries.addEntry(entry, session.role);
			}
			return {EntrySuccessResponse(), false};
		}
	} catch (const std::exception& e) {
		std::cerr << "Error adding entry: " << e.what() << "\n";
		return {EntryErrorResponse{e.what()}, false};
	}
}
EntryResponse LibraryService::deleteEntries(const EntryDeleteRequest& request, const Session& session) {
	try {
		// Temporarily lock deletion into a single id at a time
		if (request.ids.empty()) {
			throw std::runtime_error(" At least one entry id is required for deletion. ");
		}
		if (request.ids.size() > 1) {
			throw std::runtime_error(" Can only delete one entry at a time right now. ");
		}
		{
			std::unique_lock lock(entriesMutex);
			entries.deleteEntry(request.ids[0], session.role);
			return {EntrySuccessResponse(), false};
		}
	} catch (const std::exception& e) {
		std::cerr << "Error deleting entry: " << e.what() << "\n";
		return {EntryErrorResponse{e.what()}, false};
	}
}
EntryResponse LibraryService::searchEntries(const EntrySearchRequest& request, const Session& session) {
	try {
		{
			std::shared_lock lock(entriesMutex);
			return {EntryIdsResponse{.ids = entries.searchEntryTitles(request.searchTerm)}, false};
		}
	} catch (const std::exception& e) {
		std::cerr << "Error searching entry titles: " << e.what() << "\n";
		return {EntryErrorResponse{e.what()}, false};
	}
}
EntryResponse LibraryService::filterEntries(const EntryFilterRequest& request, const Session& session) {
	try {
		{
			std::shared_lock lock(entriesMutex);
			return {EntryIdsResponse{.ids = entries.getFilteredIDS(request.filters)}, false};
		}
	} catch (const std::exception& e) {
		std::cerr << "Error filtering entries: " << e.what() << "\n";
		return {EntryErrorResponse{e.what()}, false};
	}
}



Result<GroupId> LibraryService::createAnime(const CreateAnimeRequest& request, const Session& session) {
	{
		try {
			std::scoped_lock locks(entriesMutex, groupsMutex);

			auto groupId = groups.addGroup(request.group);

			for (auto entry : request.entries) {
				entry.ip_group = groupId;
				entries.addEntry(entry, session.role);
			}
			groups.compile(groupId,entries);
			return groupId;
		} catch (const std::exception& e) {
			return std::unexpected(ErrorResponse{LIBRARY_ERROR_CODE::ERROR_UNKOWN, std::string(e.what())});
		}
	}
}
Result<void> LibraryService::updateAnime(const UpdateAnimeRequest& request, const Session& session) {
	return std::unexpected(ErrorResponse{LIBRARY_ERROR_CODE::NOT_IMPLEMENTED, "updateAnime is not implemented."});

	try {
		std::scoped_lock locks(entriesMutex, groupsMutex);


	} catch (const std::exception& e) {
		return std::unexpected(ErrorResponse{LIBRARY_ERROR_CODE::ERROR_UNKOWN, std::string(e.what())});
	}

	return {}; // Returns the id of that anime(its group id)
}
Result<void> LibraryService::deleteAnime(const DeleteAnimeRequest& request, const Session& session) {
	{
		try {
			std::scoped_lock locks(entriesMutex, groupsMutex);

			const auto& targets = groups.getGroups(request.groups);
			
			std::vector<EntryId> members =
				targets | std::views::transform([](const auto& group) -> const auto& { return group.members; }) |
				std::views::join | std::ranges::to<std::vector<EntryId>>();
			std::ranges::for_each(members, [&](const auto& id) { entries.deleteEntry(id, session.role); });
			std::ranges::for_each(request.groups, [&](const auto& id) { groups.deleteGroup(id); });


			return {};
		} catch (const std::exception& e) {
			return std::unexpected(ErrorResponse{LIBRARY_ERROR_CODE::ERROR_UNKOWN, std::string(e.what())});
		}
	}
}
Result<std::vector<Entry>> LibraryService::addEntries(const AddEntriesRequest& request, const Session& session) {
	{
		try {
			std::scoped_lock locks(entriesMutex, groupsMutex);

			const auto& entryIds =
				request.entries | std::views::transform([](const auto& entry) -> const auto& { return entry.id; }) |
				std::ranges::to<std::vector<EntryId>>();
			groups.updateGroups(std::vector<GroupUpdate>{{request.id, UPDATE_OPERATION::ADD_VALUE, 4, anime::storage::dataType::VECU32, entryIds}});

			for (const auto& entry : request.entries) {
				entries.addEntry(entry, session.role);
			}
			groups.compile(request.id, entries);
			return request.entries;
		} catch (const std::exception& e) {
			return std::unexpected(ErrorResponse{LIBRARY_ERROR_CODE::ERROR_UNKOWN, std::string(e.what())});
		}
	}
}
Result<void> LibraryService::updateEntries(const UpdateEntriesRequest& request, const Session& session) {
	return std::unexpected(ErrorResponse{LIBRARY_ERROR_CODE::NOT_IMPLEMENTED, "updateEntries is not implemented."});
}
Result<void> LibraryService::transferEntry(const TransferEntryRequest& request, const Session& session) {
	{
		try {
			std::scoped_lock locks(entriesMutex, groupsMutex);

			GroupId old = std::get<std::uint32_t>(entries.getField(request.entry, 14)); // For once i get compile individual group
			entries.updateEntry(std::vector<EntryUpdate>{{request.entry, UPDATE_OPERATION::SET_FIELD, 14, request.group}}, session.role);

			groups.updateGroups(std::vector<GroupUpdate>{{old,UPDATE_OPERATION::REMOVE_VALUE,4,anime::storage::dataType::VECU32,request.entry},{request.group,UPDATE_OPERATION::ADD_VALUE,4,anime::storage::dataType::VECU32,request.entry}});
			groups.compile(old,entries);groups.compile(request.group,entries);
		} catch (const std::exception& e) {
			return std::unexpected(ErrorResponse{LIBRARY_ERROR_CODE::ERROR_UNKOWN, std::string(e.what())});
		}
	}
	return {};
}
Result<void> LibraryService::deleteEntries(const DeleteEntriesRequest& request, const Session& session) {
	{
		std::scoped_lock locks(entriesMutex, groupsMutex);

		auto parents = request.entries
			| std::views::transform([&](const auto id) -> GroupId {
				return std::get<GroupId>(entries.getField(id, "ip_group"));
			})
			| std::ranges::to<std::vector<GroupId>>();

		std::ranges::sort(parents);
		parents.erase(std::ranges::unique(parents).begin(), parents.end());

		for (const auto& id : request.entries) {
			try {
				entries.deleteEntry(id, session.role); // need to implement delete by vector for safe deletion and errors
			} catch (const std::exception& e) {
				return std::unexpected(ErrorResponse{LIBRARY_ERROR_CODE::INVALID_USER_PERMISSION_LEVEL, std::string(e.what())});
			}
		}

		try {
			groups.compile(parents,entries);
		} catch (const std::exception& e) {
			return std::unexpected(ErrorResponse{LIBRARY_ERROR_CODE::ERROR_UNKOWN, std::string(e.what())});
		}
	}
	return {};
}

Result<std::vector<Group>> LibraryService::getAnime(const GetAnimeRequest& request, const Session& session) {
	try {
		std::shared_lock lock(groupsMutex);
		return groups.getGroups(request.groups);
	} catch (const std::exception& e) {
		return std::unexpected(ErrorResponse{LIBRARY_ERROR_CODE::ERROR_UNKOWN, std::string(e.what())});
	}
}
Result<std::vector<Entry>> LibraryService::getEntries(const GetEntriesRequest& request, const Session& session) {
	return std::unexpected(ErrorResponse{LIBRARY_ERROR_CODE::NOT_IMPLEMENTED, "getEntries is not implemented."});

	/* Unfinished implementation draft, preserved for continued development.
	// Gets a anime via entry ids

	std::vector<Entry> res;
	return Result(); // Returns a entrys
	*/
}
Result<std::vector<GroupId>> LibraryService::searchAnime(const SearchAnimeRequest& request, const Session& session) {
	return std::unexpected(ErrorResponse{LIBRARY_ERROR_CODE::NOT_IMPLEMENTED, "searchAnime is not implemented."});

	/* Unfinished implementation draft, preserved for continued development.
	// string searches groups, currently bound to titles only

	std::vector<GroupId> res;
	return Result(); // Returns group ids of results
	*/
}
Result<std::vector<GroupId>> LibraryService::filterAnime(const FilterAnimeRequest& request, const Session& session) {
	return std::unexpected(ErrorResponse{LIBRARY_ERROR_CODE::NOT_IMPLEMENTED, "filterAnime is not implemented."});

	/* Unfinished implementation draft, preserved for continued development.
	// filters groups

	std::vector<GroupId> res;
	return Result(); // Returns the group ids of each passed option
	*/
}



} // namespace anime::storage
