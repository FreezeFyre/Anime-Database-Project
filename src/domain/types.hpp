#pragma once

#include <cstdint>

namespace anime::domain {

using UserId = std::uint32_t;
using IdentityId = std::uint32_t;
using SessionId = std::uint32_t;

enum class USER_ACCOUNT_ROLE : std::uint8_t {
	VIEWER = 0,
	EDITOR = 1,
	ADMIN = 2
};

} // namespace anime::domain
