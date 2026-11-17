#pragma once

#include <string_view>
#include <unordered_map>
#include <vector>
#include "protocol/auth_messages.hpp"
#include "protocol/message_types.hpp"
#include "databases/auth/password.hpp"
#include "net/http/http_types.hpp"
#include "net/transport/session.hpp"

namespace anime::app { struct Context; }

namespace anime::auth {

using anime::protocol::ACTION_TYPE;
using anime::protocol::AuthLoginRequest;
using anime::protocol::AuthCreateAccountRequest;
using anime::protocol::LoginSuccess;
using anime::protocol::LogoutSuccess;
using anime::protocol::AccountCreated;
using anime::protocol::OAuthRedirect;
using anime::protocol::AuthError;
using anime::protocol::AuthSuccess;
using anime::protocol::AuthResponse;
using anime::protocol::AuthOAuthStartRequest;
using anime::protocol::AuthOAuthCallbackRequest;
using anime::protocol::AuthLogoutRequest;
using anime::protocol::AuthRequest;
using anime::net::Session;
using anime::app::Context;

class Auth {
	public:
		Context& context;
		PasswordService passwordService;

		explicit Auth(Context& context);

		std::expected<LoginSuccess,AuthError> login(const AuthLoginRequest& request,const std::string& token);

		std::expected<LogoutSuccess,AuthError> logout(const std::string& token);

		std::expected<AccountCreated,AuthError> create(const AuthCreateAccountRequest& request,const std::string& tokenn);


	private:
		std::expected<LoginSuccess,AuthError> password(PasswordService& passwordService, const AuthLoginRequest& request,const std::string& token);


};

class OAuthService {
	public:
		OAuthService(Context& context);
		OAuthRedirect start(const AuthOAuthStartRequest& request,const std::string& token);
		AuthResponse callback(const AuthOAuthCallbackRequest& request,const std::string& token);
	
	private:
		Context& context;

		OAuthRedirect google(const AuthOAuthStartRequest& request,const std::string& token);
		OAuthRedirect apple(const AuthOAuthStartRequest& request, const std::string& token);
		OAuthRedirect github(const AuthOAuthStartRequest& request, const std::string& token);


};

} // namespace anime::auth

namespace anime::protocol {
using anime::net::http::HttpResponse;
class AuthHttpCodec {
	public:
		AuthRequest decode(ACTION_TYPE action, const std::vector<std::string_view>& path, const std::unordered_map<std::string, std::string>& headers, const std::vector<uint8_t>& body);
		HttpResponse encode(const AuthResponse& response);

	private:
		HttpResponse encodeResponse(const LoginSuccess& response);
		HttpResponse encodeResponse(const LogoutSuccess& response);
		HttpResponse encodeResponse(const AccountCreated& response);
		HttpResponse encodeResponse(const OAuthRedirect& response);

};

} // namespace anime::protocol

namespace anime::app {
using anime::protocol::ACTION_TYPE;
using anime::protocol::AuthLoginRequest;
using anime::protocol::AuthCreateAccountRequest;
using anime::protocol::AuthOAuthStartRequest;
using anime::protocol::AuthOAuthCallbackRequest;
using anime::protocol::AuthLogoutRequest;
using anime::protocol::AuthRequest;
using anime::protocol::OAuthRedirect;
using anime::protocol::AuthResponse;
class AuthRouter {
	public:
		AuthResponse route(const AuthRequest& request, Context& context, const std::string& token);

	private:
		AuthResponse handle(const AuthLoginRequest& request, Context& context, const std::string& token);
		AuthResponse handle(const AuthOAuthStartRequest& request, Context& context, const std::string& token);
		AuthResponse handle(const AuthOAuthCallbackRequest& request, Context& context, const std::string& token);
		AuthResponse handle(const AuthCreateAccountRequest& request, Context& context, const std::string& token);
		AuthResponse handle(const AuthLogoutRequest& request, Context& context, const std::string& token);

};

} // namespace anime::app
