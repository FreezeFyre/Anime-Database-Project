#pragma once

#include "domain/entry.hpp"
#include "filtering/filterProcessor.hpp"
#include "io/file_types.hpp"

namespace anime::protocol {

using anime::domain::Entry;
using anime::domain::EntryId;
using anime::domain::EntryUpdate;
using anime::storage::schemaData;
using anime::storage::sectionHeader;
using anime::filtering::FilterRequest;

struct Update {
	std::uint32_t id;
	anime::schema::UPDATE_OPERATION operation;
	std::uint8_t fieldIndex;
	anime::storage::dataType type;
	anime::storage::recordField value;
};

struct EntryGetRequest {
	std::vector<EntryId> ids;
};
struct EntryUpdateRequest {
	std::vector<EntryUpdate> updates;
};
struct EntryCreateRequest { // No schema as its used only for translation
	std::vector<Entry> entries;
};
struct EntryDeleteRequest {
	std::vector<EntryId> ids;
};
struct EntrySearchRequest { // This will change once searches arent title only
	std::string searchTerm;
};
struct EntryFilterRequest {
	FilterRequest filters;
};
using EntryRequest = std::variant<
	EntryGetRequest,
	EntryUpdateRequest,
	EntryCreateRequest,
	EntryDeleteRequest,
	EntrySearchRequest,
	EntryFilterRequest
>;

struct EntryGetResponse {
	sectionHeader section;
	schemaData schema;
	std::vector<Entry> entries;
};
struct EntryIdsResponse {
    std::vector<EntryId> ids;
};
struct EntrySuccessResponse {
    std::string message = "SUCCESS";
};

struct EntryErrorResponse {
    std::string message;
};
using EntryResponsePayload = std::variant<
    EntryGetResponse,
    EntryIdsResponse,
    EntrySuccessResponse,
    EntryErrorResponse
>;
struct EntryResponse {
	EntryResponsePayload payload;
    bool closeConnection = false;
};

} // namespace anime::protocol
