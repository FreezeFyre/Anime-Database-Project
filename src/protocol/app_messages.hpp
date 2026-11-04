#pragma once

#include <variant>
#include "protocol/message_types.hpp"
#include "protocol/entry_messages.hpp"
#include "protocol/auth_messages.hpp"
#include "protocol/library_messages.hpp"

namespace anime::protocol {

using AppPayload = std::variant<
	LibraryRequest,
	EntryRequest,
	AuthRequest
>;

using ResponsePayload = std::variant<
	LibraryResponse,
	EntryResponse,
	AuthResponse
>;

struct AppRequest {
	COM_TYPE type;
	DOMAIN_TYPE domain;
	std::uint8_t action;
	uint32_t id;
	AppPayload payload;
};

struct AppResponse {
	uint32_t id;
	ResponsePayload payload;
	bool closeConnection = false;
};

} // namespace anime::protocol
