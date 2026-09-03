#pragma once

#include <cstdint>

namespace anime::app {

enum class LOG_LEVEL : std::uint8_t {
	Error,
	Warn,
	Info,
	Debug,
	Trace
};

} // namespace anime::app
