#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>
#include <stdexcept>

#include "domain/types.hpp"
#include "schema/field.hpp"

namespace anime::domain {
using namespace anime::schema;
using namespace anime::storage;
enum class LoginProvider : uint8_t {
	PASSWORD = 0,
	GOOGLE = 1,
	APPLE = 2,
	GITHUB = 3
};
inline LoginProvider login_provider(std::string_view provider) {
	if (provider == "password") {
		return LoginProvider::PASSWORD;
	} else if (provider == "google") {
		return LoginProvider::GOOGLE;
	} else if (provider == "apple") {
		return LoginProvider::APPLE;
	} else if (provider == "github") {
		return LoginProvider::GITHUB;
	}
	throw std::runtime_error("Unknown login provider: " + std::string(provider));
}

struct User {
	UserId id = 0;
	std::string displayName;
	USER_ACCOUNT_ROLE role = USER_ACCOUNT_ROLE::VIEWER;
	bool disabled = false;
};

struct UserIdentity {
	IdentityId id = 0;
	UserId userId = 0;
	LoginProvider provider = LoginProvider::PASSWORD;
	std::string loginKey;
	std::string email;
	std::string providerUserId;
	std::string passwordHash;
	bool disabled = false;
};


using UserMemPtr = std::variant<UserId User::*, std::string User::*, USER_ACCOUNT_ROLE User::*, bool User::*>;

struct UserField {
	FieldInfo info;
	UserMemPtr ptr;
};

inline const std::array<UserField, 4> UserMeta = { {
	{{"id",            schemaField::dataType::U32, schemaField::filterMode::FM_NONE, FIELD_PRESENCE::REQUIRED,       VALUE_REFERENCE_TYPE::NONE, std::nullopt, ""}, &User::id          },
	{{"display_name",  schemaField::dataType::STR, schemaField::filterMode::FM_NONE, FIELD_PRESENCE::FIELD_OPTIONAL, VALUE_REFERENCE_TYPE::NONE, std::nullopt, ""}, &User::displayName },
	{{"role",          schemaField::dataType::U8,  schemaField::filterMode::FM_NONE, FIELD_PRESENCE::REQUIRED,       VALUE_REFERENCE_TYPE::NONE, std::nullopt, ""}, &User::role        },
	{{"disabled",      schemaField::dataType::U8,  schemaField::filterMode::FM_NONE, FIELD_PRESENCE::REQUIRED,       VALUE_REFERENCE_TYPE::NONE, std::nullopt, ""}, &User::disabled    }
} };

inline const std::unordered_map<std::string, size_t> userNameMap = [] {
	std::unordered_map<std::string, size_t> result;
	for (size_t i=0; i<UserMeta.size(); ++i) result.emplace(UserMeta[i].info.name, i);
	return result;
}();

using UserIdentityMemPtr = std::variant<uint32_t UserIdentity::*, LoginProvider UserIdentity::*, std::string UserIdentity::*, bool UserIdentity::*>;
struct IdentityField {
	FieldInfo info;
	UserIdentityMemPtr ptr;
};
inline const std::array<IdentityField, 8> UserIdentityMeta = {
	{{{"id",               schemaField::dataType::U32, schemaField::filterMode::FM_NONE, FIELD_PRESENCE::REQUIRED,       VALUE_REFERENCE_TYPE::NONE, std::nullopt, ""},&UserIdentity::id             },
	 {{"user_id",          schemaField::dataType::U32, schemaField::filterMode::FM_NONE, FIELD_PRESENCE::REQUIRED,       VALUE_REFERENCE_TYPE::NONE, std::nullopt, ""},&UserIdentity::userId         },
	 {{"provider",         schemaField::dataType::U8,  schemaField::filterMode::FM_NONE, FIELD_PRESENCE::REQUIRED,       VALUE_REFERENCE_TYPE::NONE, std::nullopt, ""},&UserIdentity::provider       },
	 {{"login_key",        schemaField::dataType::STR, schemaField::filterMode::FM_NONE, FIELD_PRESENCE::REQUIRED,       VALUE_REFERENCE_TYPE::NONE, std::nullopt, ""},&UserIdentity::loginKey       },
	 {{"email",            schemaField::dataType::STR, schemaField::filterMode::FM_NONE, FIELD_PRESENCE::FIELD_OPTIONAL, VALUE_REFERENCE_TYPE::NONE, std::nullopt, ""},&UserIdentity::email          },
	 {{"provider_user_id", schemaField::dataType::STR, schemaField::filterMode::FM_NONE, FIELD_PRESENCE::FIELD_OPTIONAL, VALUE_REFERENCE_TYPE::NONE, std::nullopt, ""},&UserIdentity::providerUserId },
	 {{"password_hash",    schemaField::dataType::STR, schemaField::filterMode::FM_NONE, FIELD_PRESENCE::FIELD_OPTIONAL, VALUE_REFERENCE_TYPE::NONE, std::nullopt, ""},&UserIdentity::passwordHash   },
	 {{"disabled",         schemaField::dataType::U8,  schemaField::filterMode::FM_NONE, FIELD_PRESENCE::REQUIRED,       VALUE_REFERENCE_TYPE::NONE, std::nullopt, ""},&UserIdentity::disabled       }
	}};
inline const std::unordered_map<std::string, size_t> identityNameMap = [] {
	std::unordered_map<std::string, size_t> result;
	for (size_t i=0; i<UserIdentityMeta.size(); ++i) result.emplace(UserIdentityMeta[i].info.name, i);
	return result;
}();

} // namespace anime::domain

