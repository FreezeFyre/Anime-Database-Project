#pragma once

#include <shared_mutex>
#include "databases/entry_database.hpp"
#include "databases/user_database.hpp"
#include "databases/group_database.hpp"
#include "filtering/filterProcessor.hpp"

namespace anime::app {
using anime::storage::entryDB;
using anime::storage::groupDB;
using anime::storage::userDB;
using anime::filtering::filterProcessor;

struct Context {
	entryDB entries;
	groupDB groups;
	filterProcessor filterService;
	userDB users;

	mutable std::shared_mutex entriesMutex;
	mutable std::shared_mutex groupsMutex;
	mutable std::shared_mutex usersMutex;

};

} // namespace anime::app
