#include "auth_service.hpp"




AuthResult Auth::login(const LoginRequest& request) {
	switch (request.provider) {
		case LoginProvider::PASSWORD: {
			return password(request.loginKey, request.secret);
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


AuthResult Auth::password(PasswordService& passwordService, const LoginRequest& request) {
	AuthResult result;
	result.success = false;
	result.role = UserRole::VIEWER;

	try {
		IdentityId identityId = context.userDB.getIdentityId_LoginKey(request.loginKey);
		const UserIdentity& identity = context.userDB.getIdentity(identityId);
		const User& user = context.userDB.getUser(identity.userId);

		if (identity.disabled || user.disabled) {
			result.error = "Invalid login.";
			return result;
		}

		if (!passwordService.verifyPassword(request.secret, identity.passwordHash)) {
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


LoginRequest AuthRouter::readLogin(const uint8_t& bytes) {
	LoginRequest result;
	ByteReader r;

	r.input = bytes;
	result.provider = static_cast<LoginProvider>(r.U8());
	result.loginKey = r.STR;
	result.secret = r.STR;
	
	return result;
}


RouteResult AuthRouter::route(Message& message, Context& context) {
	RouteResult result;

	MessageCodec codec;
	ByteReader reader{ message.payload };
	std::vector<uint8_t> buffer;

	switch (message.action) {
		case ACTION_TYPE::ACT_LOGIN: {

			break;
		}
		case ACTION_TYPE::ACT_LOGOUT: { 
		
			break;
		}
		case ACTION_TYPE::ACT_CREATE: { 
		
			break;
		}
		default:
	}


	return result;
}
