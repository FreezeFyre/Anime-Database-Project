#pragma once

#include "../domain/entry.hpp"
#include "../domain/group.hpp"
#include "io/file_types.hpp"
#include "protocol/entry_messages.hpp"
#include "filtering/filterProcessor.hpp"

#include <expected>

namespace anime::protocol {

enum class LIBRARY_ERROR_CODE : std::uint8_t {
	ERROR_UNKOWN = 0,
	NOT_IMPLEMENTED = 1,
	INVALID_USER_PERMISSION_LEVEL = 100
};

struct ErrorResponse {
	LIBRARY_ERROR_CODE code;
	std::string err;
};
template <typename T>
using Result = std::expected<T, ErrorResponse>;

using FilterAnimeRequest = anime::filtering::FilterRequest;
using anime::domain::GroupId;
using anime::domain::Group;
struct CreateAnimeRequest {
	Group group;
	std::vector<Entry> entries;
};
struct UpdateAnimeRequest {
	std::vector<Update> updates;
};
struct DeleteAnimeRequest {
	std::vector<GroupId> groups;
};

struct AddEntriesRequest {
	GroupId id;
	std::vector<Entry> entries;
};
struct UpdateEntriesRequest {
	std::vector<Update> updates;
};
struct TransferEntryRequest {
	EntryId entry;
	GroupId group;
};
struct DeleteEntriesRequest {
	std::vector<EntryId> entries;
};
struct GetAnimeRequest {
	std::vector<GroupId> groups;
};
struct GetEntriesRequest {
	std::vector<EntryId> entries;
};
struct SearchAnimeRequest {
	std::string target;
};

// error response = return std::unexpected<ErrorResponse>({code, "Error Messsage"});
}
