#pragma once

#include <optional>

#include "../core.hpp"
#include "../users.hpp"
#include "password.hpp"
#include "../../net/protocol/data.hpp"
#include "../../net/protocol/messenger.hpp"

struct Context;
struct Message;
struct RouteResult;

struct AuthResult {
	bool success;
	std::optional<UserId> userId;
	UserRole role;
	std::string error;
};

struct LoginRequest {
	LoginProvider provider;
	std::string loginKey;
	std::string secret;
};


class Auth {
	public:
		Context* context;
	    PasswordService passwordService;

		AuthResult login(const LoginRequest& request);


	private:
		AuthResult password(PasswordService& passwordService, const LoginRequest& request);
		AuthResult google(std::string_view token);
		AuthResult apple(std::string_view token);
		AuthResult github(std::string_view token);

};

class AuthRouter {
	public:
		RouteResult route(Message& message, Context& context);

	private:
		LoginRequest readLogin(const uint8_t& bytes);
};
