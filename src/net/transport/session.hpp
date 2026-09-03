#pragma once

#include <optional>
#include "domain/types.hpp"

namespace anime::net {
using namespace anime::domain;
struct Session {
	bool signedIn = false;
	std::optional<UserId> userId;
	USER_ACCOUNT_ROLE role = USER_ACCOUNT_ROLE::VIEWER;
};

} // namespace anime::net
