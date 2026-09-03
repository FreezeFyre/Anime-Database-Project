#pragma once

#include <cstdint>
#include <string_view>

namespace anime::protocol {

inline constexpr std::uint32_t ENCODE_VERSION = 1001;

enum class COM_TYPE : std::uint8_t { REQ = 1, RESP = 2, STATUS = 3 };

enum class ACTION_TYPE : std::uint8_t {
	ACT_GET = 1, ACT_UPDATE = 2, ACT_CREATE = 3, ACT_DELETE = 4,
	ACT_SEARCH = 5, ACT_SORT = 6, ACT_APPLY = 7, ACT_FILTER = 8,
	ACT_LOGIN = 9, ACT_OAUTH = 10, ACT_LOGOUT = 11, ACT_UNKNOWN = 0
};

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
	ENTRY = 1, GROUPS = 2, CHARACTERS = 3, FILTERS = 4,
	USERS = 9, AUTH = 10, UNKNOWN = 0
};

inline DOMAIN_TYPE domain_type(std::string_view type) {
	if (type == "entry") return DOMAIN_TYPE::ENTRY;
	if (type == "groups") return DOMAIN_TYPE::GROUPS;
	if (type == "characters") return DOMAIN_TYPE::CHARACTERS;
	if (type == "filters") return DOMAIN_TYPE::FILTERS;
	if (type == "users") return DOMAIN_TYPE::USERS;
	if (type == "auth") return DOMAIN_TYPE::AUTH;
	return DOMAIN_TYPE::UNKNOWN;
}

} // namespace anime::protocol
