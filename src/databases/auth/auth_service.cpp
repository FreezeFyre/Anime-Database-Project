#include "auth_service.hpp"

#include "../../net/routing/router.hpp"

Auth::Auth(Context& context) : context(context) {}

AuthResult Auth::login(const LoginRequest& request) {
	switch (request.provider) {
		case LoginProvider::PASSWORD: {
			return password(passwordService, request);
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

AuthResult Auth::create(const CreateAccountRequest& request) {
	switch (request.provider) {
		case LoginProvider::PASSWORD: {
			context.users.createAccount(request, passwordService.hash(request.secret));
			return {true, std::nullopt, UserRole::VIEWER, ""};
		}
		default:
			return {false, std::nullopt, UserRole::VIEWER, "Unsupported login provider."};
	}
}


AuthResult Auth::password(PasswordService& passwordService, const LoginRequest& request) {
	AuthResult result;
	result.success = false;
	result.role = UserRole::VIEWER;

	try {
		IdentityId identityId = context.users.getIdentityId_LoginKey(request.loginKey);
		const UserIdentity& identity = context.users.getIdentity(identityId);
		const User& user = context.users.getUser(identity.userId);

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
		return result;
	}
	catch (const std::runtime_error& e) {
		std::clog << e.what() << "\n";
		result.error = "Invalid login.";
		return result;
	}
}




RouteResult AuthRouter::route(Message& message, Context& context) {
	RouteResult result;

	auth.context = context;

	MessageCodec codec;
	ByteReader reader{ message.payload };
	std::vector<uint8_t> buffer;

	switch (message.action) {
		case ACTION_TYPE::ACT_LOGIN: {
			std::vector<uint8_t> authResult = serializer.authResult(auth.login(serializer.readLogin(message.payload)));
			break;
		}
		case ACTION_TYPE::ACT_LOGOUT: { 
			std::vector<uint8_t> authResult = serializer.authResult(auth.logout());
			break;
		}
		case ACTION_TYPE::ACT_CREATE: { 
			std::vector<uint8_t> authResult = serializer.authResult(auth.create(serializer.readCreateAccount(message.payload)));
			break;
		}
		default: {
			break;
		}
	}


	return result;
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
