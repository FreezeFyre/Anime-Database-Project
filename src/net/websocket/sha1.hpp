#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace anime::net::websocket {

std::array<uint8_t, 20> sha1Bytes(const std::vector<uint8_t>& bytes);
std::array<uint8_t, 20> sha1Bytes(const std::string& text);

} // namespace anime::net::websocket
