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
#include <shared_mutex>
#include <mutex>
#include <chrono>

#include "io/fileHandling.hpp"
#include "protocol/auth_requests.hpp"

#include "domain/user.hpp"

namespace anime::storage {
using namespace anime::domain;
using anime::protocol::AuthCreateAccountRequest;
using namespace anime::schema;

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
		void elevateUser(UserId id, USER_ACCOUNT_ROLE newRole, const USER_ACCOUNT_ROLE& role);
		void addIdentity(const UserIdentity& identity);
	    void createAccount(const AuthCreateAccountRequest& request, const std::string& argon_password);

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
	    std::string savePath;

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

} // namespace anime::storage
