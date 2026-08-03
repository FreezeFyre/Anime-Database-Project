#include "auth_service.hpp"

#include "../../net/routing/router.hpp"

Auth::Auth(Context& context) : context(context) {}

AuthResult Auth::login(const AuthLoginRequest& request, Session& session) {
	switch (request.provider) {
		case LoginProvider::PASSWORD: {
			return password(passwordService, request, session);
			break;
		}
		//case LoginProvider::GOOGLE:
		//	return google(request.secret);
		//case LoginProvider::APPLE:
		//	return apple(request.secret);
		//case LoginProvider::GITHUB:
		//	return github(request.secret);
		default:
			return {false, std::nullopt, UserRole::VIEWER, "Unsupported login provider."};
	}
}
AuthResult Auth::logout(Session& session) {
	session.signedIn = false;
	session.userId = std::nullopt;
	session.role = UserRole::VIEWER;
    return {true, session.userId, session.role, ""};
}
AuthResult Auth::create(const AuthCreateAccountRequest& request, Session& session) {
	switch (request.provider) {
		case LoginProvider::PASSWORD: {
			const std::string pw = passwordService.hash(request.secret);

			// Exclusive Mutex Lock
			{
				std::unique_lock lock(context.usersMutex);
				context.users.createAccount(request, pw);
			}
			return {true, std::nullopt, UserRole::VIEWER, ""};
		}
		default:
			return {false, std::nullopt, UserRole::VIEWER, "Unsupported login provider."};
	}
}
AuthResult Auth::password(PasswordService& passwordService, const AuthLoginRequest& request, Session& session) {
	AuthResult result;
	result.success = false;
	result.role = UserRole::VIEWER;

	try {
		IdentityId identityId;
		UserIdentity identity;
		User user;
		// Shared Mutex Lock
		{
			std::shared_lock lock(context.usersMutex);
			identityId = context.users.getIdentityId_LoginKey(request.loginKey);
			identity = context.users.getIdentity(identityId);
			user = context.users.getUser(identity.userId);
		}

		if (identity.disabled || user.disabled) {
			result.error = "Invalid login.";
			return result;
		}

		if (!passwordService.verifyPassword(request.secret, identity.passwordHash.c_str())) {
			result.error = "Invalid login.";
			return result;
		}

		result.success = true;
		result.userId = user.id;
		result.role = user.role;
		session.role = user.role;
		session.signedIn = true;
		session.userId = user.id;

		auto now = std::chrono::system_clock::now();
		std::clog << now << "\t" << "[INFO] user login success, User id: " << user.id << "\n ";


		return result;
	}
	catch (const std::runtime_error& e) {
		std::clog << e.what() << "\n";
		result.error = "Invalid login.";
		return result;
	}
}


AuthResponse AuthRouter::route(const AuthRequest& request, Context& context, Session& session) {
	return std::visit([&](const auto& payload) {
		return handle(payload, context, session);
	}, request);
}

AuthResponse AuthRouter::handle(const AuthLoginRequest& request, Context& context, Session& session) {
	return Auth{context}.login(request, session);
}
AuthResponse AuthRouter::handle(const AuthOAuthStartRequest& request, Context& context, Session& session) {
	return AuthResult{false, std::nullopt, UserRole::VIEWER, "OAuth start not implemented."};
}
AuthResponse AuthRouter::handle(const AuthOAuthCallbackRequest& request, Context& context, Session& session) {
	return AuthResult{false, std::nullopt, UserRole::VIEWER, "OAuth callback not implemented."};
}
AuthResponse AuthRouter::handle(const AuthCreateAccountRequest& request, Context& context, Session& session) {
	return Auth{context}.create(request, session);
}
AuthResponse AuthRouter::handle(const AuthLogoutRequest& request, Context& context, Session& session) {
	return Auth{context}.logout(session);
}



AuthRequest AuthHttpCodec::decode(ACTION_TYPE action, const std::vector<std::string_view>& path, const std::unordered_map<std::string, std::string>& headers, const std::vector<uint8_t>& body) {
	switch (action) {
		case ACTION_TYPE::ACT_LOGIN: {
			ByteReader reader{body};

			LoginProvider provider = login_provider(std::string(path[2]));

			std::string loginKey = reader.STR();
			std::string secret = reader.STR();

			return AuthLoginRequest{provider, loginKey, secret};
		}
		case ACTION_TYPE::ACT_OAUTH: {
			// Oauth isnt implemented yet
			std::clog << "Oauth action is not implemented yet.\n";
			throw std::runtime_error("Oauth action is not implemented yet.");
		}
		case ACTION_TYPE::ACT_CREATE: {
			ByteReader reader{body};

			LoginProvider provider = login_provider(std::string(path[2]));

			std::string loginKey = reader.STR();
			std::string secret = reader.STR();
			std::string email = reader.STR();
			std::string displayName = reader.STR();

			return AuthCreateAccountRequest{provider, loginKey, secret, email, displayName};
			break;
		}
		case ACTION_TYPE::ACT_LOGOUT: {
			return AuthLogoutRequest{};
			break;
		}
		default:
			std::cerr << "Unknown action type for auth: " << static_cast<int>(action) << "\n";
			throw std::runtime_error("Unknown action type for auth.");
	}

	return AuthRequest();
}
HttpResponse AuthHttpCodec::encode(const AuthResponse& response) {
	return encodeResponse(response);
}

HttpResponse AuthHttpCodec::encodeResponse(const AuthResponse& response) {
	if (response.success) {
		return HttpResponse{
			"HTTP/1.1",
			200,
			"OK",
			{{"Content-Type", "application/octet-stream"}},
			{static_cast<uint8_t>(response.role)}
		};
	}

	return HttpResponse {
		"HTTP/1.1",
		401,
		"Unauthorized",
		{{"Content-Type", "application/octet-stream"}},
		ByteVector{response.error.begin(), response.error.end()}
	};
}
