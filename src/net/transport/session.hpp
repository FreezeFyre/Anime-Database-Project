#pragma once

#include <optional>
#include "../../databases/users.hpp"

using SessionId = uint32_t;

struct Session {
	bool signedIn = false;
	std::optional<UserId> userId;
	UserRole role = UserRole::VIEWER;
};
