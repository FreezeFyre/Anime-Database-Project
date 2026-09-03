#pragma once

#include <optional>
#include <shared_mutex>
#include <mutex>

#include "domain/types.hpp"
#include "schema/field.hpp"
#include "protocol/message_types.hpp"
#include "protocol/auth_requests.hpp"
#include "net/http/http_types.hpp"
#include "databases/users.hpp"
#include "databases/auth/password.hpp"
#include "net/protocol/data.hpp"
#include "net/transport/session.hpp"

namespace anime::app { struct Context; }

namespace anime::auth {
using namespace anime::domain;
using namespace anime::schema;
using namespace anime::storage;
using namespace anime::protocol;
using anime::net::Session;
using anime::app::Context;
using anime::net::http::HttpResponse;


struct AuthResult {
	bool success;
	std::optional<UserId> userId;
	USER_ACCOUNT_ROLE role;
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

class OAuthService {
	public:
		OAuthService(Context& context);
		OAuthRedirect start(const AuthOAuthStartRequest& request, Session& session);
		AuthResponse callback(const AuthOAuthCallbackRequest& request, Session& session);
	
	private:
		Context& context;

		OAuthRedirect google(const AuthOAuthStartRequest& request, Session& session);
		OAuthRedirect apple(const AuthOAuthStartRequest& request, Session& session);
		OAuthRedirect github(const AuthOAuthStartRequest& request, Session& session);


};

class AuthHttpCodec {
	public:
		AuthRequest decode(ACTION_TYPE action, const std::vector<std::string_view>& path, const std::unordered_map<std::string, std::string>& headers, const std::vector<uint8_t>& body);
		HttpResponse encode(const AuthResponse& response);

	private:
	    HttpResponse encodeResponse(const AuthResult& response);
		HttpResponse encodeResponse(const OAuthRedirect& response);

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

} // namespace anime::auth
