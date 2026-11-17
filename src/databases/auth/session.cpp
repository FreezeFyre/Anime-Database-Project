#include "../../net/websocket/token_generation.hpp"
#include "session.hpp"

#include <chrono>
#include <domain/types.hpp>
#include <expected>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <mutex>


namespace anime::app::sessions {
using anime::net::getUniqueToken;
constexpr auto kSessionDuration = std::chrono::days{7}; // One week

std::string Sessions::create(UserId user) {
	std::string token = getUniqueToken();
	std::unique_lock lock(_mutex);

	const auto now = std::chrono::steady_clock::now();
	_sessions.try_emplace(token,Session{
		.authenticated = false,
		.userId = user,
		.session_token = token,
		.expiresAt = now + kSessionDuration
	});
	
	return token;
}

std::expected<Sessions::Session, Sessions::SessionError> Sessions::resolve(std::string_view token) {
	std::shared_lock lock(_mutex);
	try {
		const Sessions::Session& session = _sessions.at(std::string(token));
		if (std::chrono::steady_clock::now() < session.expiresAt) {
			return session;
		} else {
			return std::unexpected(SessionError{" Session has expired. "});
		}
	} catch (const std::out_of_range& e) {
		return std::unexpected(SessionError{std::string(e.what())});
	}
}

void Sessions::revoke(std::string_view token) {
	std::unique_lock lock(_mutex);
	_sessions.erase(std::string(token));
}

size_t Sessions::expireSessions() {

	return size_t();
}

} // namespace anime::app::sessions
