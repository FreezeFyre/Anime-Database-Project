#pragma once

#include <optional>
#include <string>
#include <variant>
#include <expected>
#include "domain/types.hpp"
#include "protocol/auth_requests.hpp"

namespace anime::protocol {
using anime::domain::UserId;
using anime::domain::USER_ACCOUNT_ROLE;
using anime::domain::LoginProvider;

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


struct LoginSuccess {
	UserId user;
	std::string token;
};
struct LogoutSuccess {};
struct AccountCreated {
	UserId user;
};
struct OAuthRedirect {
	std::string url;
};
struct AuthError {
	std::string error;
};
using AuthSuccess = std::variant<LoginSuccess, LogoutSuccess, AccountCreated, OAuthRedirect>;
using AuthResponse = std::expected<AuthSuccess, AuthError>;


} // namespace anime::protocol
