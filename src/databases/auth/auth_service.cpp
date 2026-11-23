#include "databases/auth/auth_service.hpp"
#include "app/context.hpp"
#include "net/protocol/data.hpp"
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <expected>

namespace anime::auth {
using namespace anime::domain;
using namespace anime::protocol;
Auth::Auth(Context& context) : context(context) {}

std::expected<LoginSuccess,AuthError> Auth::login(const AuthLoginRequest& request,const std::string& token) {
	switch (request.provider) {
		case LoginProvider::PASSWORD: {
			return password(passwordService, request, token);
			break;
		}
		//case LoginProvider::GOOGLE:
		//	return google(request.secret);
		//case LoginProvider::APPLE:
		//	return apple(request.secret);
		//case LoginProvider::GITHUB:
		//	return github(request.secret);
		default:
			return {std::unexpected(AuthError{"Unsupported login provider."})};
	}
}
std::expected<LogoutSuccess,AuthError> Auth::logout(const std::string& token) {
	try {
		context.sessions.revoke(token);
	} catch (const std::exception& e) {
		return std::unexpected(AuthError{std::string(e.what())});
	}
	return LogoutSuccess{};
}
std::expected<AccountCreated,AuthError> Auth::create(const AuthCreateAccountRequest& request,const std::string& token) {
	switch (request.provider) {
		case LoginProvider::PASSWORD: {
			const std::string pw = passwordService.hash(request.secret);

			// Exclusive Mutex Lock
			{
				std::unique_lock lock(context.usersMutex);
				return AccountCreated{context.users.createAccount(request, pw)};
			}
		}
		default:
			return std::unexpected(AuthError{"Unsupported login provider."});
	}
}
std::expected<LoginSuccess,AuthError> Auth::password(PasswordService& passwordService, const AuthLoginRequest& request,const std::string& token) {
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
			return std::unexpected(AuthError{"Invalid login. "});
		}

		if (!passwordService.verifyPassword(request.secret, identity.passwordHash.c_str())) {
			return std::unexpected(AuthError{"Invalid login. "});
		}


		auto token = context.sessions.create(user.id);
		return LoginSuccess{user.id, token};
	}
	catch (const std::runtime_error& e) {
		std::clog << e.what() << "\n";
		return std::unexpected(AuthError{"Invalid login. "});
	}
}


OAuthService::OAuthService(Context& context) : context(context) {}

OAuthRedirect OAuthService::start(const AuthOAuthStartRequest& request, const std::string& token) {
	switch (request.provider) {
		case LoginProvider::GOOGLE: {
			return OAuthRedirect{google(request, token)};
		}
		case LoginProvider::APPLE: {
			return OAuthRedirect{apple(request, token)};
		}
		case LoginProvider::GITHUB: {
			return OAuthRedirect{github(request, token)};
		}
		default:
			std::clog << "Unsupported OAuth provider: " << static_cast<int>(request.provider) << "\n";
			throw std::runtime_error("Unsupported OAuth provider: " + std::to_string(static_cast<int>(request.provider)));
			break;
	}
}
AuthResponse OAuthService::callback(const AuthOAuthCallbackRequest& request, const std::string& token) {
	// Handle callback

	// Based on the provider we need to contact the correct OAuth service to exchange the code for an access token and
	// user info.


	return AuthResponse();
}

OAuthRedirect OAuthService::google(const AuthOAuthStartRequest& request, const std::string& token) {
	return OAuthRedirect();
}
OAuthRedirect OAuthService::apple(const AuthOAuthStartRequest& request, const std::string& token) {
	return OAuthRedirect();
}
OAuthRedirect OAuthService::github(const AuthOAuthStartRequest& request, const std::string& token) {
	return OAuthRedirect();
}

} // namespace anime::auth

namespace anime::app {
using namespace anime::auth;
AuthResponse AuthRouter::route(const AuthRequest& request, Context& context, const std::string& token) {
	return std::visit([&](const auto& payload) {
		return handle(payload, context, token);
	}, request);
}

AuthResponse AuthRouter::handle(const AuthLoginRequest& request, Context& context, const std::string& token) {
	return Auth{context}.login(request, token);
}
AuthResponse AuthRouter::handle(const AuthOAuthStartRequest& request, Context& context, const std::string& token) {
	return OAuthService{context}.start(request, token);
}
AuthResponse AuthRouter::handle(const AuthOAuthCallbackRequest& request, Context& context, const std::string& token) {
	return OAuthService{context}.callback(request, token);
}
AuthResponse AuthRouter::handle(const AuthCreateAccountRequest& request, Context& context, const std::string& token) {
	return Auth{context}.create(request, token);
}
AuthResponse AuthRouter::handle(const AuthLogoutRequest& request, Context& context, const std::string& token) {
	return Auth{context}.logout(token);
}



} // namespace anime::app

namespace anime::protocol {
AuthRequest AuthHttpCodec::decode(ACTION_TYPE action, const std::vector<std::string_view>& path, const std::vector<uint8_t>& body) {
	switch (action) {
		case ACTION_TYPE::ACT_LOGIN: {
			ByteReader reader{body};

			LoginProvider provider = anime::domain::login_provider(std::string(path[2]));

			std::string loginKey = reader.read<std::string>();
			std::string secret = reader.read<std::string>();

			return AuthLoginRequest{provider, loginKey, secret};
		}
		case ACTION_TYPE::ACT_OAUTH: {
			// Oauth isnt implemented yet
			std::clog << "Oauth action is not implemented yet.\n";
			throw std::runtime_error("Oauth action is not implemented yet.");
		}
		case ACTION_TYPE::ACT_CREATE: {
			ByteReader reader{body};

			LoginProvider provider = anime::domain::login_provider(std::string(path[2]));

			std::string loginKey = reader.read<std::string>();
			std::string secret = reader.read<std::string>();
			std::string email = reader.read<std::string>();
			std::string displayName = reader.read<std::string>();

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
	if (response.has_value()) {
		return std::visit([&](const auto& resp) { return encodeResponse(resp); }, *response);	
	} else {
		const auto& message = response.error().error;
		return HttpResponse {
			"HTTP/1.1",
			401,
			"Unauthorized",
			{{"Content-Type", "application/octet-stream"}},
			ByteVector{message.begin(), message.end()}
		};
	}
}

HttpResponse AuthHttpCodec::encodeResponse(const LoginSuccess& response) {
	return HttpResponse{
		"HTTP/1.1",
		200,
		"OK",
		{{"Set-Cookie", "session=" + response.token + "; Path=/; HttpOnly; SameSite=Lax"}},
		{}
	};
}
HttpResponse AuthHttpCodec::encodeResponse(const LogoutSuccess& response) {
	return HttpResponse{
		"HTTP/1.1",
		204,
		"No Content",
		{{"Set-Cookie",
		  "session=; Path=/; Max-Age=0; HttpOnly; SameSite=Lax"}},
		{}
	};
}
HttpResponse AuthHttpCodec::encodeResponse(const AccountCreated& response) {
	ByteWriter writer;
	writer.write(response.user);

	return HttpResponse{
		"HTTP/1.1",
		201,
		"Created",
		{{"Content-Type", "application/octet-stream"}},
		std::move(writer.buffer)
	};
}

HttpResponse AuthHttpCodec::encodeResponse(const OAuthRedirect& response) {
	return HttpResponse{
		"HTTP/1.1",
		302,
		"Found",
		{{"Location", response.url}},
		{}
	};
}



} // namespace anime::protocol
