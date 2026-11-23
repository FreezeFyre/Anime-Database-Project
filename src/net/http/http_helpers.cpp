#include "http_helpers.hpp"

#include <string>
#include <string_view>
#include <vector>
#include <utility>
#include <stdexcept>
#include <ranges>

namespace anime::net::http {

Headers parseHeaders(std::string_view headers) {
	Headers result;

	while (!headers.empty()) {
		const auto lineEnd = headers.find("\r\n");
		const auto line = headers.substr(0, lineEnd);

		if (lineEnd == std::string_view::npos) {
			headers = {};
		} else {
			headers.remove_prefix(lineEnd + 2);
		}

		// An empty line ends the header section.
		if (line.empty()) {
			break;
		}

		const auto colon = line.find(':');
		if (colon == std::string_view::npos || colon == 0) {
			throw std::runtime_error("Invalid HTTP header: missing name or colon.");
		}

		std::string key{line.substr(0, colon)};

		for (char& character : key) {
			const bool letter =
				(character >= 'A' && character <= 'Z') ||
				(character >= 'a' && character <= 'z');
			const bool digit = character >= '0' && character <= '9';
			const bool punctuation =
				std::string_view{"!#$%&'*+-.^_`|~"}.find(character) !=
				std::string_view::npos;

			if (!letter && !digit && !punctuation) {
				throw std::runtime_error("Invalid character in HTTP header name.");
			}

			if (character >= 'A' && character <= 'Z') {
				character = static_cast<char>(character + ('a' - 'A'));
			}
		}

		auto value = line.substr(colon + 1);

		for (const unsigned char character : value) {
			if ((character < 0x20 && character != '\t') || character == 0x7F) {
				throw std::runtime_error("Invalid character in HTTP header value.");
			}
		}

		const auto first = value.find_first_not_of(" \t");
		if (first == std::string_view::npos) {
			value = {};
		} else {
			const auto last = value.find_last_not_of(" \t");
			value = value.substr(first, last - first + 1);
		}

		result.emplace_back(std::move(key), std::string{value});
	}

	return result;
}


std::vector<std::string> getHeaderValues( const Headers& headers, std::string_view header ) {
	return headers
		| std::views::filter([&](const auto& pair) {
			return pair.first == header;
		})
		| std::views::transform([](const auto& pair) {
			return pair.second;
		})
		| std::ranges::to<std::vector<std::string>>();
}

std::string retrieveCookie(const Headers& headers, std::string_view target) {
	std::string result{};
	bool found = false;

	for (const auto& value : getHeaderValues(headers, "cookie")) {
		std::string_view window = value;

		while (!window.empty()) {
			auto pos = window.find(';');
			auto cookie = window.substr(0, pos);

			if (pos == std::string_view::npos) {
				window = {};
			} else {
				window.remove_prefix(pos + 1);
			}

			constexpr std::string_view whitespace = " \t";
			auto first = cookie.find_first_not_of(whitespace);
			if (first == std::string_view::npos) {continue;}
			auto last = cookie.find_last_not_of(whitespace);
			cookie = cookie.substr(first, last - first + 1);

			auto equals = cookie.find('=');
			if (equals == std::string_view::npos) {continue;}

			auto title = cookie.substr(0, equals);
			if (title != target) {continue;}

			if (found) {
				throw std::runtime_error("Duplicate cookie: " + std::string(target));
			}

			found = true;
			result = std::string(cookie.substr(equals + 1));
		}
	}

	return result;
}

} // namespace anime::net::http
