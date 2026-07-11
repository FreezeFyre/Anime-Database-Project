#include "password.hpp"

std::string PasswordService::hash(const std::string& password) {
	std::array<uint8_t, SALT_LENGTH> salt;
	randomSalt(salt.data() salt.size());
	size_t ENCODE_LENGTH = argon2_encodedlen(TIME_COST, MEMORY_KIB, PARALLELISM, SALT_LENGTH, HASH_LENGTH, Argon2_id);

	std::vector<char> encoded(ENCODE_LENGTH);
	if (argon2id_hash_encoded(TIME_COST, MEMORY_KIB,PARALLELISM, password.data(), password.size(), salt.data(), salt.size(), HASH_LENGTH, encoded.data(), encoded.size()) != ARGON2_OK) {
		throw std::runtime_error("Failed to hash password");
	}
	
	return std::string(encoded.data());
}

void PasswordService::randomSalt(uint8_t* data, size_t size) {
	NTSTATUS status = BCryptGenRandom(nullptr, data, static_cast<ULONG>(size), BCRYPT_USE_SYSTEM_PREFERRED_RNG);

	if (status < 0) {
		throw std::runtime_error("BCryptGenRandom failed.");
	}
}

bool PasswordService::verifyPassword(const std::string inPassword, const std::string hashedPassword) {
	if (argon2d_verify(hashedPassword, inPassword, inPassword.length()) == ARGON2_OK) {
		return true;
	}
	return false;
}



