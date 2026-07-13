#pragma once

#include <optional>

#include "../../core.hpp"
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


class Auth {
	public:
		Context& context;
		PasswordService passwordService;

		explicit Auth(Context& context);

		AuthResult login(const LoginRequest& request);

		AuthResult create(const CreateAccountRequest& request);


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
		LoginRequest readLogin(const std::vector<uint8_t>& bytes);
};

class AuthSerializer {
	public:

};

// Possible auth messages are Logging in, creating a account, and logging out,
// a login request should be a Uint8_t for provider then a string that holds the login key and a string that holds the secret
// a create account request should include a uin8_t for provider, a string for login key, a string for secret and a optional string for email
// Log-out dosent require packet, can just do clear off action
