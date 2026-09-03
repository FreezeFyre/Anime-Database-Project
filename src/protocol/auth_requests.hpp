#pragma once

#include <string>
#include <variant>
#include "domain/user.hpp"

namespace anime::protocol {
using anime::domain::LoginProvider;

struct AuthLoginRequest {
	LoginProvider provider;
	std::string loginKey;
	std::string secret;
};
struct AuthCreateAccountRequest {
	LoginProvider provider;
	std::string loginKey;
	std::string secret;
	std::string email; // Optional, only used for password provided login. Must be requested for providers.
	std::string displayName;
};

using AuthRequests = std::variant<
	AuthLoginRequest,
	AuthCreateAccountRequest
>;


} // namespace anime::protocol
