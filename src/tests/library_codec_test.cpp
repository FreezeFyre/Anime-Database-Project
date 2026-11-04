#include "protocol/codec.hpp"
#include <cassert>

using namespace anime::protocol;

int main() {
	LibraryWSC codec;
	const ByteVector ids{0, 0, 0, 1, 0, 0, 1, 2};
	assert(std::get<GetAnimeRequest>(codec.decode(LIBRARY_ACTION::GET_ANIME, ids)).groups.at(0) == 258);
	assert(std::get<GetEntriesRequest>(codec.decode(LIBRARY_ACTION::GET_ENTRIES, ids)).entries.at(0) == 258);
	assert(std::get<DeleteAnimeRequest>(codec.decode(LIBRARY_ACTION::DELETE_ANIME, ids)).groups.at(0) == 258);
	assert(std::get<DeleteEntriesRequest>(codec.decode(LIBRARY_ACTION::DELETE_ENTRIES, ids)).entries.at(0) == 258);
	const auto transfer =
	    std::get<TransferEntryRequest>(codec.decode(LIBRARY_ACTION::TRANSFER_ENTRY, {0, 0, 0, 7, 0, 0, 0, 8}));
	assert(transfer.entry == 7 && transfer.group == 8);

	ByteVector entry{0, 0, 0, 1, 'T'};
	entry.resize(38); // Remaining fields have zero values or empty prefixes.
	ByteVector create{0, 0, 0, 1, 'G', 0, 0, 0, 0, 0, 0, 0, 1};
	create.insert(create.end(), entry.begin(), entry.end());
	const auto created = std::get<CreateAnimeRequest>(codec.decode(LIBRARY_ACTION::CREATE_ANIME, create));
	assert(created.group.name == "G" && created.entries.at(0).title == "T");
	ByteVector add{0, 0, 0, 9, 0, 0, 0, 1};
	add.insert(add.end(), entry.begin(), entry.end());
	const auto added = std::get<AddEntriesRequest>(codec.decode(LIBRARY_ACTION::ADD_ENTRIES, add));
	assert(added.id == 9 && added.entries.at(0).title == "T");

	const ByteVector updates{0, 0, 0, 1, 0, 0, 0, 7, 0, 2, 4, 0, 0, 0, 0, 0, 0, 1, 0};
	const auto updated = std::get<UpdateEntriesRequest>(codec.decode(LIBRARY_ACTION::UPDATE_ENTRIES, updates));
	assert(std::get<std::uint64_t>(updated.updates.at(0).value) == 256);
	ByteVector both{0, 0, 0, 0};
	both.insert(both.end(), updates.begin(), updates.end());
	const auto combined = std::get<UpdateAnimeRequest>(codec.decode(LIBRARY_ACTION::UPDATE_ANIME, both));
	assert(combined.groupUpdates.empty() && combined.entryUpdates.size() == 1);

	const auto rejects = [&](LIBRARY_ACTION action, const ByteVector& bytes) {
		try {
			(void)codec.decode(action, bytes);
		} catch (const std::runtime_error&) {
			return;
		}
		throw std::runtime_error("Malformed library request was accepted.");
	};
	rejects(LIBRARY_ACTION::GET_ANIME, {0, 0, 0, 1});
	rejects(LIBRARY_ACTION::GET_ANIME, {0, 0, 0, 0, 1});
	rejects(LIBRARY_ACTION::UPDATE_ENTRIES, {255, 255, 255, 255});
	rejects(static_cast<LIBRARY_ACTION>(255), {});

	assert((codec.encode(Result<GroupId>{258}) == ByteVector{1, 0, 0, 1, 2}));
	assert((codec.encode(Result<void>{}) == ByteVector{1}));
	assert((codec.encode(Result<std::vector<GroupId>>{std::vector<GroupId>{258}}) ==
	        ByteVector{1, 0, 0, 0, 1, 0, 0, 1, 2}));
	const ByteVector emptyList{1, 0, 0, 0, 0};
	assert(codec.encode(Result<std::vector<GroupId>>{std::vector<GroupId>{}}) == emptyList);
	assert(codec.encode(Result<std::vector<Entry>>{std::vector<Entry>{}}) == emptyList);
	assert(codec.encode(Result<std::vector<Group>>{std::vector<Group>{}}) == emptyList);
	Entry outputEntry;
	outputEntry.title = "T";
	ByteVector entryResponse{1, 0, 0, 0, 1}; // Status and count.
	entryResponse.resize(13);                // ID and rank.
	entryResponse.insert(entryResponse.end(), entry.begin(), entry.end());
	entryResponse.resize(entryResponse.size() + 4); // Parent group ID.
	assert(codec.encode(Result<std::vector<Entry>>{std::vector<Entry>{outputEntry}}) == entryResponse);
	const ErrorResponse error{LIBRARY_ERROR_CODE::NOT_IMPLEMENTED, "x"};
	const ByteVector failure{0, 1, 0, 0, 0, 1, 'x'};
	assert(codec.encode(error) == failure);
	assert(codec.encode(Result<void>{std::unexpected(error)}) == failure);
	assert(codec.encode(Result<GroupId>{std::unexpected(error)}) == failure);
	assert(codec.encode(Result<std::vector<GroupId>>{std::unexpected(error)}) == failure);
	assert(codec.encode(Result<std::vector<Entry>>{std::unexpected(error)}) == failure);
	assert(codec.encode(Result<std::vector<Group>>{std::unexpected(error)}) == failure);

	Group group;
	group.id = 7;
	group.name = "G";
	group.members = {258};
	const ByteVector groupResponse{
	    1, 0, 0, 0, 1,                                                // Status and group count.
	    0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 1, 'G', 0, 0, 0, 0,          // ID, rank, name, description.
	    0, 0, 0, 1, 0, 0, 1, 2, 0, 0, 0, 0,                           // Members and genres.
	    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, // Runtime and counts.
	    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 1                 // Levels, completeness, revision, version.
	};
	assert(codec.encode(Result<std::vector<Group>>{std::vector<Group>{group}}) == groupResponse);
}
