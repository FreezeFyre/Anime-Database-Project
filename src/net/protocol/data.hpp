#include <string>
#include <vector>
#include <optional>
#include <cstdint>

#pragma once

#ifdef OPTIONAL
#undef OPTIONAL
#endif


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
	void VECU32(const std::vector<uint32_t>& vec);
	void VECU16(const std::vector<int>& vec);
	void VECU8(const std::vector<int>& vec);


	template<typename T>
	void OPTIONAL(const std::optional<T>& opt);
};


class ByteReader {
public:
	const std::vector<uint8_t>& input;
	size_t offset = 0;

	uint8_t U8();
	uint16_t U16();
	uint32_t U32();
	float F32();
	float F64();
	char CHAR();
	std::string STR();
	std::vector<int> VECU8();
	std::vector<int> VECU16();
	std::vector<size_t> VECU32();

	template<typename T>
	std::optional<T> OPTIONAL();
};
