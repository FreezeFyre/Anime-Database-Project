#pragma once

#include <cstdint>
#include <string_view>

namespace anime::protocol {

inline constexpr std::uint32_t ENCODE_VERSION = 1002;

enum class COM_TYPE : std::uint8_t { REQ = 1, RESP = 2, STATUS = 3 };

enum class ACTION_TYPE : std::uint8_t {
	ACT_GET = 1, ACT_UPDATE = 2, ACT_CREATE = 3, ACT_DELETE = 4,
	ACT_SEARCH = 5, ACT_SORT = 6, ACT_APPLY = 7, ACT_FILTER = 8,
	ACT_LOGIN = 9, ACT_OAUTH = 10, ACT_LOGOUT = 11, ACT_UNKNOWN = 0
};

enum class LIBRARY_ACTION : std::uint8_t {
	GET_ANIME = 1,
	GET_ENTRIES = 2,
	CREATE_ANIME = 3,
	ADD_ENTRIES = 4,
	UPDATE_ANIME = 5,
	UPDATE_ENTRIES = 6,
	TRANSFER_ENTRY = 7,
	DELETE_ANIME = 8,
	DELETE_ENTRIES = 9,
	UNKNOWN = 0
};
inline LIBRARY_ACTION library_action(std::string_view type) {
	if (type == "get_anime") return      LIBRARY_ACTION::GET_ANIME;
	if (type == "get_entries") return    LIBRARY_ACTION::GET_ENTRIES;
	if (type == "create_anime") return   LIBRARY_ACTION::CREATE_ANIME;
	if (type == "add_entries") return    LIBRARY_ACTION::ADD_ENTRIES;
	if (type == "update_anime") return   LIBRARY_ACTION::UPDATE_ANIME;
	if (type == "update_entries") return LIBRARY_ACTION::UPDATE_ENTRIES;
	if (type == "transfer_entry") return LIBRARY_ACTION::TRANSFER_ENTRY;
	if (type == "delete_anime") return   LIBRARY_ACTION::DELETE_ANIME;
	if (type == "delete_entries") return LIBRARY_ACTION::DELETE_ENTRIES;
	return LIBRARY_ACTION::UNKNOWN;
}

inline ACTION_TYPE action_type(std::string_view type) {
	if (type == "get") return ACTION_TYPE::ACT_GET;
	if (type == "update") return ACTION_TYPE::ACT_UPDATE;
	if (type == "create") return ACTION_TYPE::ACT_CREATE;
	if (type == "delete") return ACTION_TYPE::ACT_DELETE;
	if (type == "search") return ACTION_TYPE::ACT_SEARCH;
	if (type == "sort") return ACTION_TYPE::ACT_SORT;
	if (type == "apply") return ACTION_TYPE::ACT_APPLY;
	if (type == "filter") return ACTION_TYPE::ACT_FILTER;
	if (type == "login") return ACTION_TYPE::ACT_LOGIN;
	if (type == "oauth") return ACTION_TYPE::ACT_OAUTH;
	if (type == "logout") return ACTION_TYPE::ACT_LOGOUT;
	return ACTION_TYPE::ACT_UNKNOWN;
}

enum class DOMAIN_TYPE : std::uint8_t {
	ENTRY = 1, GROUPS = 2, CHARACTERS = 3, LIBRARY = 4, FILTERS = 5,
	USERS = 9, AUTH = 11, UNKNOWN = 0
}; // Databases are 1-10, auth are 11-20, filters is left but can probably be discarded

inline DOMAIN_TYPE domain_type(std::string_view type) {
	if (type == "entry") return DOMAIN_TYPE::ENTRY;
	if (type == "groups") return DOMAIN_TYPE::GROUPS;
	if (type == "library") return DOMAIN_TYPE::LIBRARY;
	if (type == "characters") return DOMAIN_TYPE::CHARACTERS;
	if (type == "filters") return DOMAIN_TYPE::FILTERS;
	if (type == "users") return DOMAIN_TYPE::USERS;
	if (type == "auth") return DOMAIN_TYPE::AUTH;
	return DOMAIN_TYPE::UNKNOWN;
}

} // namespace anime::protocol
