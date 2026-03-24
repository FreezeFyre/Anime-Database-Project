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

void ByteWriter::String(const std::string& str) {
	U32(str.size());
	for (char c : str) {
		CHAR(c);
	}
}

void ByteWriter::VEC32(const std::vector<uint32_t>& vec) {
	U32(vec.size());
	for (uint32_t value : vec) {
		U32(value);
	}
}