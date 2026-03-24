#include <string>
#include <vector>
#include <optional>
#include <cstdint>

#pragma once


class ByteWriter {
public:
	std::vector<uint8_t> buffer;

	void U8(uint8_t value);
	void U16(uint16_t value);
	void U32(uint32_t value); 
	void F32(float value);
	void F64(float value);
	void CHAR(char c);
	void String(const std::string& str);
	void VEC32(const std::vector<uint32_t>& vec);

	template<typename T>
	void writeOptional(const std::optional<T>& opt);
};


class ByteReader {
public:
	const std::vector<uint8_t>& buffer;
	size_t offset = 0;

	uint32_t readU32();
	std::string readString();
	std::vector<uint32_t> readVector();

	template<typename T>
	std::optional<T> readOptional();
};