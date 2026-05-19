#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

std::string base64Encode(const std::vector<uint8_t>& bytes);
std::string base64Encode(const uint8_t* bytes, size_t size);
std::string base64Encode(const std::array<uint8_t, 20>& bytes);
