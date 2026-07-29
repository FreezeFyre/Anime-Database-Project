#include "base64.hpp"

std::string base64Encode(const uint8_t* bytes, size_t size) {
	static constexpr char alphabet[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";

	std::string output;
	output.reserve(((size + 2) / 3) * 4);

	for (size_t i = 0; i < size; i += 3) {
		const uint32_t b0 = bytes[i];
		const uint32_t b1 = (i + 1 < size) ? bytes[i + 1] : 0;
		const uint32_t b2 = (i + 2 < size) ? bytes[i + 2] : 0;
		const uint32_t triple = (b0 << 16) | (b1 << 8) | b2;

		output.push_back(alphabet[(triple >> 18) & 0x3F]);
		output.push_back(alphabet[(triple >> 12) & 0x3F]);
		output.push_back((i + 1 < size) ? alphabet[(triple >> 6) & 0x3F] : '=');
		output.push_back((i + 2 < size) ? alphabet[triple & 0x3F] : '=');
	}

	return output;
}

std::string base64Encode(const std::vector<uint8_t>& bytes) {
	return base64Encode(bytes.data(), bytes.size());
}

std::string base64Encode(const std::array<uint8_t, 20>& bytes) {
	return base64Encode(bytes.data(), bytes.size());
}
