#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <stdexcept>

#include "base64.hpp"

#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#else 
#include <sys/random.h>
#include <cerrno>
#endif // _WIN32


namespace anime::net {

constexpr auto kTokenSize = 32;

using anime::net::websocket::base64UrlEncode;

inline std::string getSecureRandom(size_t size) {
	std::string buffer(size,'\0');
	#ifdef _WIN32
	NTSTATUS status = BCryptGenRandom(
		nullptr,
		reinterpret_cast<PUCHAR>(buffer.data()),
		static_cast<ULONG>(buffer.size()),
		BCRYPT_USE_SYSTEM_PREFERRED_RNG
	);
	if (!BCRYPT_SUCCESS(status)) {
		throw std::runtime_error("BCryptGenRandom failed");
	}
	#else
	std::size_t filled = 0;

	while (filled < buffer.size()) {
		const ssize_t result = getrandom(
			buffer.data() + filled,
			buffer.size() - filled,
			0
		);
	
		if (result < 0) {
			if (errno == EINTR) {
				continue;
			}
			throw std::system_error(
				errno, std::generic_category(), "getrandom failed"
			);
		}
	
		if (result == 0) {
			throw std::runtime_error("getrandom returned no bytes");
		}
	
		filled += static_cast<std::size_t>(result);
	}

	#endif
	return buffer;
}

inline std::string getUniqueToken(size_t size) {
	const auto bytes = getSecureRandom(size);

	return base64UrlEncode(
		reinterpret_cast<const std::uint8_t*>(bytes.data()),
		bytes.size()
	);
}

inline std::string getUniqueToken() {
	const auto bytes = getSecureRandom(kTokenSize);

	return base64UrlEncode(
		reinterpret_cast<const std::uint8_t*>(bytes.data()),
		bytes.size()
	);
}

}
