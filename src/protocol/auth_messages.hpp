#pragma once

#include <optional>
#include <string>
#include <variant>
#include "domain/types.hpp"
#include "protocol/auth_requests.hpp"

namespace anime::protocol {
using anime::domain::UserId;
using anime::domain::USER_ACCOUNT_ROLE;
using anime::domain::LoginProvider;

struct AuthResult {
	bool success;
	std::optional<UserId> userId;
	USER_ACCOUNT_ROLE role;
	std::string error;
};

struct AuthOAuthStartRequest {
	LoginProvider provider;
};
struct AuthOAuthCallbackRequest {
	LoginProvider provider;
	std::string code;
	std::string state;
};
struct AuthLogoutRequest {
};
using AuthRequest = std::variant<
	AuthLoginRequest,
	AuthOAuthStartRequest,
	AuthOAuthCallbackRequest,
	AuthCreateAccountRequest,
	AuthLogoutRequest
>;


struct OAuthRedirect {
	std::string url;
};
using AuthResponse = std::variant<AuthResult, OAuthRedirect>;

} // namespace anime::protocol
