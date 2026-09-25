#include "net/protocol/data.hpp"

namespace anime::protocol {




void ByteWriter::write(bool value) {
	write(static_cast<std::uint8_t>(value ? 1 : 0));
}
void ByteWriter::write(std::uint8_t value) {
	buffer.push_back(value);
}
void ByteWriter::write(std::uint16_t value) {
	buffer.push_back((value >> 8) & 0xFF);
	buffer.push_back(value & 0xFF);
}
void ByteWriter::write(std::uint32_t value){
	buffer.push_back((value >> 24) & 0xFF);
	buffer.push_back((value >> 16) & 0xFF);
	buffer.push_back((value >> 8) & 0xFF);
	buffer.push_back(value & 0xFF);
}
void ByteWriter::write(std::uint64_t value){
	buffer.push_back((value >> 56) & 0xFF);
	buffer.push_back((value >> 48) & 0xFF);
	buffer.push_back((value >> 40) & 0xFF);
	buffer.push_back((value >> 32) & 0xFF);
	buffer.push_back((value >> 24) & 0xFF);
	buffer.push_back((value >> 16) & 0xFF);
	buffer.push_back((value >> 8) & 0xFF);
	buffer.push_back(value & 0xFF);
}
void ByteWriter::write(char c) {
	buffer.push_back(static_cast<unsigned char>(c));
}
void ByteWriter::write(const std::string& str) {
	if (std::in_range<std::uint32_t>(str.size())) {
		write(static_cast<std::uint32_t>(str.size()));
	} else {
		throw std::runtime_error(" String is to large, cant have lenght over a 32bit uint. ");
	}
	for (char c : str) {
		write(c);
	}
}


} // namespace anime::protocol
