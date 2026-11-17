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
	try {
		std::scoped_lock locks(entriesMutex, groupsMutex);

		auto groupId = groups.addGroup(request.group);

		std::vector<EntryId> members = request.entries | std::views::transform([&](const auto& entry){
				auto buffer = entry;
				buffer.ip_group = groupId;
				return entries.addEntry(buffer, session.role);
			}) | std::ranges::to<std::vector<EntryId>>();


		groups.attachMembers({{groupId, members}});
		groups.compile(groupId,entries);
		return groupId;
	} catch (const std::exception& e) {
		return std::unexpected(ErrorResponse{LIBRARY_ERROR_CODE::ERROR_UNKOWN, std::string(e.what())});
	}
}
Result<void> LibraryService::updateAnime(const UpdateAnimeRequest& request, const Session& session) {
	try {
		std::scoped_lock locks(entriesMutex, groupsMutex);

		auto groupIds = groups.updateGroups(request.groupUpdates);
		auto childrenIds = entries.updateEntry(request.entryUpdates,session.role);
		auto children = entries.getEntries(childrenIds);

		auto parents = children | std::views::transform([&](const auto& entry) { return entry.ip_group; }) | std::ranges::to<std::vector<GroupId>>();

		
		groupIds.append_range(parents);
		std::ranges::sort(groupIds);
		groupIds.erase(std::ranges::unique(groupIds).begin(), groupIds.end());

		groups.compile(groupIds, entries);
		return {};
	} catch (const std::exception& e) {
		return std::unexpected(ErrorResponse{LIBRARY_ERROR_CODE::ERROR_UNKOWN, std::string(e.what())});
	}
	return {};
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
Result<std::vector<EntryId>> LibraryService::addEntries(const AddEntriesRequest& request, const Session& session) {
	{
		try {
			std::scoped_lock locks(entriesMutex, groupsMutex);

			if (!groups.contains(request.id)) { return std::unexpected(ErrorResponse{LIBRARY_ERROR_CODE::ERROR_UNKOWN, " Target group:" + std::to_string(request.id) + ", does not exist" }); }

			const auto& entryIds =
				request.entries | std::views::transform([&](const auto& entry) {auto buffer = entry; buffer.ip_group = request.id; return entries.addEntry(buffer, session.role); }) |
				std::ranges::to<std::vector<EntryId>>();

			auto members = groups.getGroups({request.id}).front().members;
			members.insert(members.end(), entryIds.begin(), entryIds.end());
			groups.attachMembers({{request.id, std::move(members)}});
			groups.compile(request.id, entries);
			return entryIds;
		} catch (const std::exception& e) {
			return std::unexpected(ErrorResponse{LIBRARY_ERROR_CODE::ERROR_UNKOWN, std::string(e.what())});
		}
	}
}
Result<void> LibraryService::updateEntries(const UpdateEntriesRequest& request, const Session& session) {
	try {
		std::scoped_lock locks(entriesMutex, groupsMutex);
		const auto& members = entries.updateEntry(request.updates,session.role);
		auto parents = members
			| std::views::transform([&](const auto id) -> GroupId { return std::get<GroupId>(entries.getField(id, "ip_group")); })
			| std::ranges::to<std::vector<GroupId>>();
		groups.compile(parents, entries);
		return {};
	} catch (const std::exception& e) {
		return std::unexpected(ErrorResponse{LIBRARY_ERROR_CODE::ERROR_UNKOWN, std::string(e.what())});
	}

}
Result<void> LibraryService::transferEntry(const TransferEntryRequest& request, const Session& session) {
	{
		try {
			if (session.role < USER_ACCOUNT_ROLE::EDITOR) {
				return std::unexpected(ErrorResponse{LIBRARY_ERROR_CODE::INVALID_USER_PERMISSION_LEVEL, " Users permission level must be editor or higher. "});
			}
			std::scoped_lock locks(entriesMutex, groupsMutex);

			if (!groups.contains(request.group)) {return std::unexpected(ErrorResponse{LIBRARY_ERROR_CODE::ERROR_UNKOWN, " Target group" + std::to_string(request.group) + "does not exist. "}); }

			std::unordered_map<GroupId, std::vector<EntryId>> members;
			GroupId old = std::get<std::uint32_t>(entries.getField(request.entry, 14));
			members[old] = std::get<std::vector<std::uint32_t>>(groups.getField(old, 4)) |
				           std::views::filter([&](std::uint32_t id) { return id != request.entry; }) |
				           std::ranges::to<std::vector>();

			entries.setMembership(request.group, request.entry);

			members.try_emplace(
				request.group, std::get<std::vector<EntryId>>(groups.getField(request.group, 4))
			).first->second.push_back(request.entry);

			groups.attachMembers(members);
			groups.compile(std::vector<GroupId>{old,request.group}, entries);
		} catch (const std::exception& e) {
			return std::unexpected(ErrorResponse{LIBRARY_ERROR_CODE::ERROR_UNKOWN, std::string(e.what())});
		}
	}
	return {};
}
Result<void> LibraryService::deleteEntries(const DeleteEntriesRequest& request, const Session& session) {
	try {


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

		std::unordered_set<EntryId> removals(request.entries.begin(), request.entries.end());
		std::unordered_map<GroupId, std::vector<EntryId>> memberships;
		for (const Group& group : groups.getGroups(parents)) {
			memberships.emplace(group.id, group.members);
		}
		for (auto& [parent, members] : memberships) {
			std::erase_if(members, [&](EntryId id) { return removals.contains(id); });
		}

		try {
			groups.attachMembers(memberships);
			groups.compile(parents,entries);
		} catch (const std::exception& e) {
			return std::unexpected(ErrorResponse{LIBRARY_ERROR_CODE::ERROR_UNKOWN, std::string(e.what())});
		}
	} catch (const std::exception& e) {
		return std::unexpected(ErrorResponse{LIBRARY_ERROR_CODE::ERROR_UNKOWN, std::string(e.what())});
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
	try {
		std::shared_lock lock(entriesMutex);

		return entries.getEntries(request.entries);
	} catch (const std::exception& e) {
		return std::unexpected(ErrorResponse{LIBRARY_ERROR_CODE::ERROR_UNKOWN, std::string(e.what())});
	}
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
