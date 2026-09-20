#include "databases/user_database.hpp"
#include "databases/auth/password.hpp"

#include <cassert>
#include <iostream>
#include <string>

namespace {

void hashesAndVerifiesPassword() {
	anime::auth::PasswordService passwords;
	std::string hash = passwords.hash("correct horse battery staple");

	assert(hash.rfind("$argon2id$", 0) == 0);
	assert(passwords.verifyPassword("correct horse battery staple", hash.c_str()));
	assert(!passwords.verifyPassword("wrong password", hash.c_str()));
}

void saltsAreUnique() {
	anime::auth::PasswordService passwords;
	std::string first = passwords.hash("same password");
	std::string second = passwords.hash("same password");

	assert(first != second);
	assert(passwords.verifyPassword("same password", first.c_str()));
	assert(passwords.verifyPassword("same password", second.c_str()));
}

void userDatabaseStoresEncodedHash() {
	anime::storage::userDB users;
	anime::auth::PasswordService passwords;
	const anime::domain::UserId id = 1;
	users.addUser({id, "freez", anime::domain::USER_ACCOUNT_ROLE::ADMIN, false});
	anime::domain::UserIdentity identity;
	identity.id = 1;
	identity.userId = id;
	identity.loginKey = "freez";
	identity.passwordHash = passwords.hash("password123");
	users.addIdentity(identity);
	assert(users.getUser(id).displayName == "freez");
	const auto& stored = users.getIdentity(anime::domain::IdentityId{1});
	assert(stored.passwordHash.rfind("$argon2id$", 0) == 0);
	assert(passwords.verifyPassword("password123", stored.passwordHash.c_str()));
	assert(!passwords.verifyPassword("bad-password", stored.passwordHash.c_str()));
}

}

int main() {
	hashesAndVerifiesPassword();
	saltsAreUnique();
	userDatabaseStoresEncodedHash();

	std::cout << "auth_test passed\n";
	return 0;
}
