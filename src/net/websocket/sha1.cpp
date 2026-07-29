#include "sha1.hpp"

#include <cstddef>

namespace {
uint32_t rotateLeft(uint32_t value, uint32_t bits) {
	return (value << bits) | (value >> (32 - bits));
}

void appendU64BigEndian(std::vector<uint8_t>& bytes, uint64_t value) {
	for (int shift = 56; shift >= 0; shift -= 8) {
		bytes.push_back(static_cast<uint8_t>((value >> shift) & 0xFF));
	}
}

void writeU32BigEndian(std::array<uint8_t, 20>& output, size_t offset, uint32_t value) {
	output[offset] = static_cast<uint8_t>((value >> 24) & 0xFF);
	output[offset + 1] = static_cast<uint8_t>((value >> 16) & 0xFF);
	output[offset + 2] = static_cast<uint8_t>((value >> 8) & 0xFF);
	output[offset + 3] = static_cast<uint8_t>(value & 0xFF);
}
}

std::array<uint8_t, 20> sha1Bytes(const std::vector<uint8_t>& bytes) {
	std::vector<uint8_t> message = bytes;
	const uint64_t originalBitLength = static_cast<uint64_t>(message.size()) * 8;

	message.push_back(0x80);
	while ((message.size() % 64) != 56) {
		message.push_back(0x00);
	}
	appendU64BigEndian(message, originalBitLength);

	uint32_t h0 = 0x67452301;
	uint32_t h1 = 0xEFCDAB89;
	uint32_t h2 = 0x98BADCFE;
	uint32_t h3 = 0x10325476;
	uint32_t h4 = 0xC3D2E1F0;

	for (size_t chunk = 0; chunk < message.size(); chunk += 64) {
		uint32_t words[80] = {};

		for (size_t i = 0; i < 16; ++i) {
			const size_t offset = chunk + i * 4;
			words[i] =
				(static_cast<uint32_t>(message[offset]) << 24) |
				(static_cast<uint32_t>(message[offset + 1]) << 16) |
				(static_cast<uint32_t>(message[offset + 2]) << 8) |
				static_cast<uint32_t>(message[offset + 3]);
		}

		for (size_t i = 16; i < 80; ++i) {
			words[i] = rotateLeft(words[i - 3] ^ words[i - 8] ^ words[i - 14] ^ words[i - 16], 1);
		}

		uint32_t a = h0;
		uint32_t b = h1;
		uint32_t c = h2;
		uint32_t d = h3;
		uint32_t e = h4;

		for (size_t i = 0; i < 80; ++i) {
			uint32_t f = 0;
			uint32_t k = 0;

			if (i < 20) {
				f = (b & c) | ((~b) & d);
				k = 0x5A827999;
			}
			else if (i < 40) {
				f = b ^ c ^ d;
				k = 0x6ED9EBA1;
			}
			else if (i < 60) {
				f = (b & c) | (b & d) | (c & d);
				k = 0x8F1BBCDC;
			}
			else {
				f = b ^ c ^ d;
				k = 0xCA62C1D6;
			}

			const uint32_t temp = rotateLeft(a, 5) + f + e + k + words[i];
			e = d;
			d = c;
			c = rotateLeft(b, 30);
			b = a;
			a = temp;
		}

		h0 += a;
		h1 += b;
		h2 += c;
		h3 += d;
		h4 += e;
	}

	std::array<uint8_t, 20> digest = {};
	writeU32BigEndian(digest, 0, h0);
	writeU32BigEndian(digest, 4, h1);
	writeU32BigEndian(digest, 8, h2);
	writeU32BigEndian(digest, 12, h3);
	writeU32BigEndian(digest, 16, h4);
	return digest;
}

std::array<uint8_t, 20> sha1Bytes(const std::string& text) {
	return sha1Bytes(std::vector<uint8_t>(text.begin(), text.end()));
}
