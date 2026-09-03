#pragma once

#include <shared_mutex>
#include "databases/entry_data.hpp"
#include "databases/users.hpp"
#include "filtering/filterProcessor.hpp"

namespace anime::app {
using anime::storage::entryDB;
using anime::storage::userDB;
using anime::filtering::filterProcessor;

struct Context {
	entryDB entries;
	filterProcessor filterService;
	userDB users;

	mutable std::shared_mutex entriesMutex;
	mutable std::shared_mutex usersMutex;
};

} // namespace anime::app
