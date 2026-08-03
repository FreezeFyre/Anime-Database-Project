#pragma once

#include <optional>
#include <shared_mutex>
#include <mutex>

#include "../../core.hpp"
#include "../users.hpp"
#include "password.hpp"
#include "../../net/protocol/data.hpp"
#include "../../net/transport/session.hpp"

struct Context;
struct Session;

struct AuthResult {
	bool success;
	std::optional<UserId> userId;
	UserRole role;
	std::string error;
};

class Auth {
	public:
		Context& context;
		PasswordService passwordService;

		explicit Auth(Context& context);

		AuthResult login(const AuthLoginRequest& request, Session& session);

		AuthResult logout(Session& session);

		AuthResult create(const AuthCreateAccountRequest& request, Session& session);


	private:
		AuthResult password(PasswordService& passwordService, const AuthLoginRequest& request, Session& session);
		AuthResult google(std::string_view token, Session& session);
		AuthResult apple(std::string_view token, Session& session);
		AuthResult github(std::string_view token, Session& session);

};

struct AuthLoginRequest {
	LoginProvider provider;
	std::string loginKey;
	std::string secret;
};
struct AuthOAuthStartRequest {
	LoginProvider provider;
};
struct AuthOAuthCallbackRequest {
	LoginProvider provider;
	std::string code;
	std::string state;
};
struct AuthCreateAccountRequest {
	LoginProvider provider;
	std::string loginKey;
	std::string secret;
	std::string email; // Optional, only used for password provided login. Must be requested for providers.
	std::string displayName;
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

using AuthResponse = AuthResult;


class AuthHttpCodec {
	public:
		AuthRequest decode(ACTION_TYPE action, const std::vector<std::string_view>& path, const std::unordered_map<std::string, std::string>& headers, const std::vector<uint8_t>& body);
		HttpResponse encode(const AuthResponse& response);

	private:
	    HttpResponse encodeResponse(const AuthResponse& response);
};

class AuthRouter {
	public:
		AuthResponse route(const AuthRequest& request, Context& context, Session& session);

	private:
		AuthResponse handle(const AuthLoginRequest& request, Context& context, Session& session);
		AuthResponse handle(const AuthOAuthStartRequest& request, Context& context, Session& session);
		AuthResponse handle(const AuthOAuthCallbackRequest& request, Context& context, Session& session);
		AuthResponse handle(const AuthCreateAccountRequest& request, Context& context, Session& session);
		AuthResponse handle(const AuthLogoutRequest& request, Context& context, Session& session);

};
