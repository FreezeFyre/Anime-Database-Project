#pragma once

#include <optional>
#include "domain/types.hpp"

namespace anime::net {
using anime::domain::UserId;
using anime::domain::USER_ACCOUNT_ROLE;
struct Session {
	bool signedIn = false;
	std::optional<UserId> userId;
	USER_ACCOUNT_ROLE role = USER_ACCOUNT_ROLE::VIEWER;
};

} // namespace anime::net
