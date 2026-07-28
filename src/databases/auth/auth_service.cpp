#include "auth_service.hpp"

#include "../../net/routing/router.hpp"

Auth::Auth(Context& context) : context(context) {}

AuthResult Auth::login(const LoginRequest& request, Session& session) {
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

AuthResult Auth::create(const CreateAccountRequest& request, Session& session) {
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


AuthResult Auth::password(PasswordService& passwordService, const LoginRequest& request, Session& session) {
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




RouteResult AuthRouter::route(Message& message, Context& context, Session& session) {
	RouteResult res;

	Auth auth{context};

	MessageCodec codec;
	ByteReader reader{ message.payload };
	std::vector<uint8_t> buffer;

	switch (message.action) {
		case ACTION_TYPE::ACT_LOGIN: {
			LoginRequest req = serializer.readLogin(message.payload);
			AuthResult result;

			// Shared Mutex lock within function to not hold on password hashing
			result = auth.login(req, session);
			
			std::vector<uint8_t> authResult = serializer.authResult(result);

			res.message = codec.construct(COM_TYPE::RESP, DOMAIN_TYPE::AUTH, message.action, message.id, authResult.size(), authResult);
			return res;
			break;
		}
		case ACTION_TYPE::ACT_LOGOUT: { 

			std::vector<uint8_t> authResult = serializer.authResult(auth.logout(session));
			res.message = codec.construct(COM_TYPE::RESP, DOMAIN_TYPE::AUTH, message.action, message.id, authResult.size(), authResult);
			return res;
			break;
		}
		case ACTION_TYPE::ACT_CREATE: { 
			CreateAccountRequest req = serializer.readCreateAccount(message.payload);
			AuthResult result;
			
			// Exclusive Mutex Lock within function to not hold on password hashing
			result = auth.create(req, session);
			std::vector<uint8_t> authResult = serializer.authResult(result);

			res.message = codec.construct(COM_TYPE::RESP, DOMAIN_TYPE::AUTH, message.action, message.id, authResult.size(), authResult);
			return res;
			break;
		}
		default: {
			break;
		}
	}


	return res;
}


LoginRequest AuthSerializer::readLogin(const std::vector<uint8_t>& bytes) {
	LoginRequest result;
	ByteReader r{bytes};

	result.provider = static_cast<LoginProvider>(r.U8());
	result.loginKey = r.STR();
	result.secret = r.STR();
	
	return result;
}

CreateAccountRequest AuthSerializer::readCreateAccount(const std::vector<uint8_t>& bytes) {
	CreateAccountRequest result;
	ByteReader r{bytes};

	result.provider = static_cast<LoginProvider>(r.U8());
	result.loginKey = r.STR();
	result.secret = r.STR();
	result.email = r.STR();
	result.displayName = r.STR();

	return result;
}


std::vector<uint8_t> AuthSerializer::authResult(const AuthResult& result)  {
	ByteWriter w;
	
	w.BOOL(result.success);
	if (result.userId.has_value()) {
		w.BOOL(true);
		w.U32(static_cast<uint32_t>(result.userId.value()));
	} else {
		w.BOOL(false);
	}
	w.U8(static_cast<uint8_t>(result.role));
	w.STR(result.error);

	return w.buffer;
}
