#include "data.hpp"


void ByteWriter::U8(uint8_t value) {
	buffer.push_back(value);
}

void ByteWriter::U16(uint16_t value) {
	buffer.push_back((value >> 8) & 0xFF);
	buffer.push_back(value & 0xFF);
}

void ByteWriter::U32(uint32_t value){
	buffer.push_back((value >> 24) & 0xFF);
	buffer.push_back((value >> 16) & 0xFF);
	buffer.push_back((value >> 8) & 0xFF);
	buffer.push_back(value & 0xFF);
}


void ByteWriter::CHAR(char c) {
	buffer.push_back(static_cast<unsigned char>(c));
}

void ByteWriter::STR(const std::string& str) {
	U32(str.size());
	for (char c : str) {
		CHAR(c);
	}
}

void ByteWriter::VECU32(const std::vector<uint32_t>& vec) {
	U32(vec.size());
	for (uint32_t value : vec) {
		U32(value);
	}
}

void ByteWriter::VECU16(const std::vector<int>& vec) {
	U32(vec.size());
	for (uint32_t value : vec) {
		U16(value);
	}
}

void ByteWriter::VECU8(const std::vector<int>& vec) {
	U32(vec.size());
	for (uint32_t value : vec) {
		U8(value);
	}
}



uint8_t ByteReader::U8() {
	uint8_t value = input[offset];
	offset += 1;
	return value;
}

uint16_t ByteReader::U16() {
	uint16_t value = 0;
	value |= static_cast<uint16_t>(input[offset]) << 8;
	value |= static_cast<uint16_t>(input[offset + 1]);
	offset += 2;
	return value;
}

uint32_t ByteReader::U32() {
	uint32_t value = 0;
	value |= static_cast<uint32_t>(input[offset]) << 24;
	value |= static_cast<uint32_t>(input[offset + 1]) << 16;
	value |= static_cast<uint32_t>(input[offset + 2]) << 8;
	value |= static_cast<uint32_t>(input[offset + 3]);
	offset += 4;
	return value;
}

std::string ByteReader::STR() {
	uint32_t length = U32();
	std::string str;
	str.reserve(length);
	for (uint32_t i = 0; i < length; i++) {
		str.push_back(CHAR());
	}
	return str;
}

char ByteReader::CHAR() {
	char c = static_cast<char>(input[offset]);
	offset += 1;
	return c;
}

std::vector<int> ByteReader::VECU8() {
	uint32_t length = U32();
	std::vector<int> vec;
	vec.reserve(length);
	for (uint32_t i = 0; i < length; i++) {
		vec.push_back(U8());
	}
	return vec;
}

std::vector<int> ByteReader::VECU16() {
	uint32_t length = U32();
	std::vector<int> vec;
	vec.reserve(length);
	for (uint32_t i = 0; i < length; i++) {
		vec.push_back(U16());
	}
	return vec;
}

std::vector<uint32_t> ByteReader::VECU32() {
	uint32_t length = U32();
	std::vector<uint32_t> vec;
	vec.reserve(length);
	for (uint32_t i = 0; i < length; i++) {
		vec.push_back(U32());
	}
	return vec;
}
