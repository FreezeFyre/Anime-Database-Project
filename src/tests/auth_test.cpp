#include "../databases/users.hpp"
#include "../users/auth.hpp"

#include <cassert>
#include <iostream>
#include <string>

namespace {

void hashesAndVerifiesPassword() {
	std::string hash = auth::hashPassword("correct horse battery staple");

	assert(hash.rfind("$argon2id$", 0) == 0);
	assert(auth::verifyPassword("correct horse battery staple", hash));
	assert(!auth::verifyPassword("wrong password", hash));
}

void saltsAreUnique() {
	std::string first = auth::hashPassword("same password");
	std::string second = auth::hashPassword("same password");

	assert(first != second);
	assert(auth::verifyPassword("same password", first));
	assert(auth::verifyPassword("same password", second));
}

void userDatabaseStoresEncodedHash() {
	userDB users;
	UserId id = users.addUser("freez", "password123", UserRole::ADMIN);

	const User* user = users.getUser(id);
	assert(user != nullptr);
	assert(user->displayName == "freez");

	const UserIdentity* identity = users.getPasswordIdentity("freez");
	assert(identity != nullptr);
	assert(identity->passwordHash.rfind("$argon2id$", 0) == 0);
	assert(users.verifyUserPassword("freez", "password123"));
	assert(!users.verifyUserPassword("freez", "bad-password"));
	assert(!users.verifyUserPassword("missing", "password123"));
}

}

int main() {
	hashesAndVerifiesPassword();
	saltsAreUnique();
	userDatabaseStoresEncodedHash();

	std::cout << "auth_test passed\n";
	return 0;
}
