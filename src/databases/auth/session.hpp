// Should handle per session data structures, such as session tokens, expiration times, and user associations.
#pragma once

#include <chrono>
#include <expected>
#include <optional>
#include <unordered_map>

#include "domain/types.hpp"

#include <string>
#include <string_view>
#include <mutex>
#include <shared_mutex>

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
			std::chrono::steady_clock::time_point expiresAt;
		};
		struct SessionError {
			std::string error;
		};

		std::string create(UserId user);
		std::expected<Session, SessionError> resolve(std::string_view token);
		void revoke(std::string_view token);
		size_t expireSessions();
	private:
		mutable std::shared_mutex _mutex;
		std::unordered_map<std::string, Session> _sessions;
};

} // namespace anime::app
