#pragma once

#include <string>
#include <vector>
#include <optional>
#include <cstdint>

#include "databases/database_ops.hpp"

namespace anime::protocol {

#ifdef OPTIONAL
#undef OPTIONAL
#endif

using ByteVector = std::vector<uint8_t>;
using anime::storage::helpers::always_false_v;
using anime::storage::helpers::is_vector_v;
using anime::storage::helpers::UnsignedInteger;
using anime::storage::helpers::UnsignedVector;

class ByteWriter { // Big endian encoding
public:
	ByteVector buffer;

	void write(bool value);
	void write(char c);
	void write(const std::string& str);
	void write(std::uint8_t value);
	void write(std::uint16_t value);
	void write(std::uint32_t value);
	void write(std::uint64_t value);
	template <UnsignedInteger T> void write(const std::vector<T>& vec) {
		if (std::in_range<std::uint32_t>(vec.size())) {
			write(static_cast<std::uint32_t>(vec.size()));
		} else {
			throw std::runtime_error(" Vector is to large, cant have lenght over a 32bit uint. ");
		}
		for (T val : vec) {
			write(val);
		}
	}


};


class ByteReader {
public:
	const std::vector<uint8_t>& input;
	size_t offset = 0;

	template <typename T> T read() {
		if constexpr (UnsignedInteger<T>) {
			if (offset > input.size() || sizeof(T) > input.size() - offset) {
				throw std::runtime_error("Not enough bytes to read this value.");
			}
			T value = 0;
			for (size_t i = 0; i < sizeof(T); ++i) {
				const auto next = input[offset++];
				value = static_cast<T>((value << 8) | next);
			}
			return value;
		} else if constexpr (UnsignedVector<T>) {
			using ElementType = typename T::value_type;
			std::uint32_t length = read<std::uint32_t>();
			if (length > (input.size() - offset) / sizeof(ElementType)) {
				throw std::runtime_error("Not enough bytes to read this vector.");
			}
			T buffer;
			buffer.reserve(length);
			for (size_t i = 0; i < length; i++) {
				buffer.push_back(read<ElementType>());
			}
			return buffer;
		} else if constexpr (std::same_as<T, std::string>) {
			std::uint32_t length = read<std::uint32_t>();
			std::string str;
			if (length > (input.size() - offset) / sizeof(char)) {
				throw std::runtime_error("Not enough bytes to read this string.");
			}
			str.reserve(length);
			for (std::uint32_t i = 0; i < length; i++) {
				str.push_back(read<char>());
			}
			return str;
		} else if constexpr (std::same_as<T, char>) {
			return static_cast<char>(read<std::uint8_t>());
		} else if constexpr (std::same_as<T, bool>) {
			return read<std::uint8_t>() == 1;
		} else {
			static_assert(always_false_v<T>, "Unsupported type for ByteReader::read");
		}
	}


};

} // namespace anime::protocol
