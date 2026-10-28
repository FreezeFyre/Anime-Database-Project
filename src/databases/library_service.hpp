#pragma once

#include "entry_database.hpp"
#include "protocol/entry_messages.hpp"
#include "protocol/library_messages.hpp"
#include "net/transport/session.hpp"

#include "group_database.hpp"

#include <mutex>
#include <shared_mutex>

namespace anime::storage {
using namespace anime::protocol;
using anime::net::Session;
using anime::storage::validation::VALIDATION_SCOPE;

// Should own entry and group databases and their mutexes, helps integrate routing and manages connections between groups and entries, essentially it does the work after authentication passes
	class LibraryService {
			entryDB& entries;
			groupDB& groups;
			std::shared_mutex& entriesMutex;
			std::shared_mutex& groupsMutex;
		public:
			LibraryService(entryDB& e, groupDB& g, std::shared_mutex& eMutex, std::shared_mutex& gMutex);

			inline bool validate(VALIDATION_SCOPE scope) {
				return validation::validate(entries, scope, groups);
			}
			
			EntryResponse getEntries(const EntryGetRequest& request, const Session& session);
			EntryResponse updateEntries(const EntryUpdateRequest& request, const Session& session);
			EntryResponse createEntries(const EntryCreateRequest& request, const Session& session);
			EntryResponse deleteEntries(const EntryDeleteRequest& request, const Session& session);
			EntryResponse searchEntries(const EntrySearchRequest& request, const Session& session);
			EntryResponse filterEntries(const EntryFilterRequest& request, const Session& session);

			Result<GroupId> createAnime   (const CreateAnimeRequest& request, const Session& session);
			Result<void> updateAnime   (const UpdateAnimeRequest& request, const Session& session);
			Result<void> deleteAnime   (const DeleteAnimeRequest& request, const Session& session);
				
			Result<std::vector<Entry>> addEntries    (const AddEntriesRequest& request, const Session& session);
			Result<void> updateEntries (const UpdateEntriesRequest& request, const Session& session);
			Result<void> transferEntry (const TransferEntryRequest& request, const Session& session);
			Result<void> deleteEntries (const DeleteEntriesRequest& request, const Session& session);
							   
			Result<std::vector<Group>> getAnime      (const GetAnimeRequest& request, const Session& session);
			Result<std::vector<Entry>> getEntries    (const GetEntriesRequest& request, const Session& session);
			Result<std::vector<GroupId>> searchAnime   (const SearchAnimeRequest& request, const Session& session);
			Result<std::vector<GroupId>> filterAnime   (const FilterAnimeRequest& request, const Session& session);
	};

	class LibraryRouter {
	};


}
