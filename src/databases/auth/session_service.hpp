#pragma once

#include <chrono>
#include <unordered_map>
#include <optional>
#include <expected>

#include "domain/types.hpp"

namespace anime::app::sessions {
using anime::domain::UserId;
using anime::domain::USER_ACCOUNT_ROLE;

class Sessions {
	public:
		struct Session {
			bool authenticated = false;
			std::optional<UserId> userId;
			USER_ACCOUNT_ROLE role = USER_ACCOUNT_ROLE::VIEWER;
			std::string session_token;
			std::chrono::system_clock::time_point expiresAt;
		};
		struct SessionError {
			std::string error;
		};

		std::string create(UserId user);
		std::expected<Session, SessionError> resolve(std::string_view token);
		void revoke(std::string_view token);
		size_t expireSessions();
	private:
		std::shared_mutex _sessionsMutex;
		std::unordered_map<std::string, Session> _sessions;
};

} // namespace anime::app
