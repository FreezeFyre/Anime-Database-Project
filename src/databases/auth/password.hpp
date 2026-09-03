#pragma once

#include <cstdint>
#include <string>


namespace anime::auth {

inline constexpr uint32_t TIME_COST = 3;
inline constexpr uint32_t MEMORY_KIB = 64 * 1024;
inline constexpr uint32_t PARALLELISM = 4;
inline constexpr uint32_t HASH_LENGTH = 32;
inline constexpr uint32_t SALT_LENGTH = 16;




class PasswordService {
	public:
	std::string hash(const std::string& password);

	bool verifyPassword(const std::string& inPassword, const char* hashedPassword);


	// Need to replace in core with a random linux+windows option
	void randomSalt(uint8_t* data, size_t size);
};

} // namespace anime::auth
