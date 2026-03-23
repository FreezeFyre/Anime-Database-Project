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