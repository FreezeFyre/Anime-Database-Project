#pragma once

#include <argon2.h>

#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <bcrypt.h>


constexpr uint32_t TIME_COST = 3;
constexpr uint32_t MEMORY_KIB = 64 * 1024;
constexpr uint32_t PARALLELISM = 4;
constexpr uint32_t HASH_LENGTH = 32;
constexpr uint32_t SALT_LENGTH = 16;




class PasswordService {
	public:
	std::string hash(const std::string& password);

	bool verifyPassword(const std::string inPassword, const std::string hashedPassword);


	// Need to replace in core with a random linux+windows option
	void randomSalt(uint8_t* data, size_t size);
};
