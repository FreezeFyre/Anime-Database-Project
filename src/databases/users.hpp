#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "../io/fileHandling.hpp"
#include "../core.hpp"

using UserId = uint32_t;
using IdentityId = uint32_t;

enum class UserRole : uint8_t {
	VIEWER = 0,
	EDITOR = 1,
	ADMIN = 2
};

enum class LoginProvider : uint8_t {
	PASSWORD = 0,
	GOOGLE = 1,
	APPLE = 2,
	GITHUB = 3
};

struct User {
	UserId id = 0;
	std::string displayName;
	UserRole role = UserRole::VIEWER;
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

struct CreateAccountRequest {
	LoginProvider provider;
	std::string loginKey;
	std::string secret;
	std::string email; // Optional, only used for password provided login. Must be requested for providers.
	std::string displayName;
};

struct LoginRequest {
	LoginProvider provider;
	std::string loginKey;
	std::string secret;
};


using UserMemPtr = std::variant<UserId User::*, std::string User::*, UserRole User::*, bool User::*>;

struct UserField {
	FieldInfo info;
	UserMemPtr ptr;
};

inline const std::array<UserField, 4> UserMeta = { {
	{{"id",            schemaField::dataType::U32, schemaField::filterMode::FM_NONE, fieldPresence::REQUIRED,       ValueRef::NONE, std::nullopt, ""}, &User::id          },
	{{"display_name",  schemaField::dataType::STR, schemaField::filterMode::FM_NONE, fieldPresence::FIELD_OPTIONAL, ValueRef::NONE, std::nullopt, ""}, &User::displayName },
	{{"role",          schemaField::dataType::U8,  schemaField::filterMode::FM_NONE, fieldPresence::REQUIRED,       ValueRef::NONE, std::nullopt, ""}, &User::role        },
	{{"disabled",      schemaField::dataType::U8,  schemaField::filterMode::FM_NONE, fieldPresence::REQUIRED,       ValueRef::NONE, std::nullopt, ""}, &User::disabled    }
} };

std::unordered_map<std::string, size_t> buildUserFieldNames(); 
inline const std::unordered_map<std::string, size_t> userNameMap = buildUserFieldNames();

using UserIdentityMemPtr = std::variant<uint32_t UserIdentity::*, LoginProvider UserIdentity::*, std::string UserIdentity::*, bool UserIdentity::*>;
struct IdentityField {
	FieldInfo info;
	UserIdentityMemPtr ptr;
};
inline const std::array<IdentityField, 8> UserIdentityMeta = {
	{{{"id",               schemaField::dataType::U32, schemaField::filterMode::FM_NONE, fieldPresence::REQUIRED,       ValueRef::NONE, std::nullopt, ""},&UserIdentity::id             },
	 {{"user_id",          schemaField::dataType::U32, schemaField::filterMode::FM_NONE, fieldPresence::REQUIRED,       ValueRef::NONE, std::nullopt, ""},&UserIdentity::userId         },
	 {{"provider",         schemaField::dataType::U8,  schemaField::filterMode::FM_NONE, fieldPresence::REQUIRED,       ValueRef::NONE, std::nullopt, ""},&UserIdentity::provider       },
	 {{"login_key",        schemaField::dataType::STR, schemaField::filterMode::FM_NONE, fieldPresence::REQUIRED,       ValueRef::NONE, std::nullopt, ""},&UserIdentity::loginKey       },
	 {{"email",            schemaField::dataType::STR, schemaField::filterMode::FM_NONE, fieldPresence::FIELD_OPTIONAL, ValueRef::NONE, std::nullopt, ""},&UserIdentity::email          },
	 {{"provider_user_id", schemaField::dataType::STR, schemaField::filterMode::FM_NONE, fieldPresence::FIELD_OPTIONAL, ValueRef::NONE, std::nullopt, ""},&UserIdentity::providerUserId },
	 {{"password_hash",    schemaField::dataType::STR, schemaField::filterMode::FM_NONE, fieldPresence::FIELD_OPTIONAL, ValueRef::NONE, std::nullopt, ""},&UserIdentity::passwordHash   },
	 {{"disabled",         schemaField::dataType::U8,  schemaField::filterMode::FM_NONE, fieldPresence::REQUIRED,       ValueRef::NONE, std::nullopt, ""},&UserIdentity::disabled       }
	}};
std::unordered_map<std::string, size_t> buildIdentityFieldNames(); 
inline const std::unordered_map<std::string, size_t> identityNameMap = buildIdentityFieldNames();

class userDB {
	public:
		void load(const std::string& filename);
		void save(const std::string& filename);

		// File Reading
		void section(const sectionHeader& section);
		void schema(const sectionHeader& section, const schemaData& schema);
		void record(const sectionHeader& section, const recordData& record, const schemaData& schema);
		void table(const sectionHeader& section, const std::string& row);

		// File Writing
		fileHeader getHeader(fileType type) const;
		sectionHeader getSection(fileType type, size_t index) const;
		schemaData getSchema(fileType type, size_t sectionIndex) const;
		recordData getRecord(fileType type, size_t sectionIndex, size_t recordIndex) const;
		recordData getTableRecord(fileType type, size_t sectionIndex, size_t recordIndex) const;


		// Mutate Operations
		void addUser(const User& user);
		void elevateUser(UserId id, UserRole newRole);
		void addIdentity(const UserIdentity& identity);
	    void createAccount(const CreateAccountRequest& request, const std::string& argon_password);

		// Query Operations
	    const User& getUser(UserId id) const;
	    const User& getUser(size_t index) const;
	    const UserIdentity& getIdentity(IdentityId id) const;
	    const UserIdentity& getIdentity(size_t index) const;
		IdentityId getIdentityId_LoginKey(const std::string& loginKey) const;

		// Indexing
		void constructUserIdMap(std::unordered_map<UserId, size_t>& userIdMap) const;
		void constructIdentityIdMap(std::unordered_map<IdentityId, size_t>& identityIdMap) const;
	    std::unordered_map<std::string, IdentityId> constructKeyMap(LoginProvider provider);


		// Validation
		bool validatePresence(const recordField& field, const FieldInfo& info) const;
		std::vector<size_t> validateUserSchema(const schemaData& schema) const;
		std::vector<size_t> validateIdentitySchema(const schemaData& schema) const;
	    bool validateUserIds() const;
	    bool validateIdentityIds() const;
	    bool validateEmail(const std::string& email) const;


		// Utilities
		recordField getUserField(size_t id, size_t fieldIndex) const;
		recordField getIdentityField(size_t id, size_t fieldIndex) const;
		recordField getFromUser(const User& user, const size_t fieldIndex) const;
		recordField getFromIdentity(const UserIdentity& identity, const size_t fieldIndex) const;

	private:
		UserId nextUserId = 1;
		IdentityId nextIdentityId = 1;
		std::vector<User> users;
		std::vector<UserIdentity> identities;
		std::unordered_map<UserId, size_t> userIdMap;
		std::unordered_map<IdentityId, size_t> identityIdMap;

		std::unordered_map<std::string, IdentityId> passwordMap;

		std::vector<size_t> userFieldMap;
		std::vector<size_t> identityFieldMap;

};
