#pragma once

#include <optional>
#include "../../core.hpp"

struct Session {
	bool signedIn = false;
	std::optional<UserId> userId;
	UserRole role = UserRole::VIEWER;
};
