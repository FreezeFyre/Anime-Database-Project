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
	void STR(const std::string& str);
	void VEC32(const std::vector<uint32_t>& vec);
	void VECU16(const std::vector<uint16_t>& vec);
	void VECU8(const std::vector<uint8_t>& vec);


	template<typename T>
	void OPTIONAL(const std::optional<T>& opt);
};


class ByteReader {
public:
	const std::vector<uint8_t>& input;
	size_t offset = 0;

	size_t U8();
	size_t U16();
	size_t U32();
	float F32();
	float F64();
	char CHAR();
	std::string STR();
	std::vector<size_t> VECU8();
	std::vector<size_t> VECU16();
	std::vector<size_t> VEC32();

	template<typename T>
	std::optional<T> OPTIONAL();
};