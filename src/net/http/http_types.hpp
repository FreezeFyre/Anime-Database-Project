#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>

#include "net/protocol/data.hpp"

#ifdef DELETE
#undef DELETE
#endif

namespace anime::net::http {

enum class HTTP_METHOD : std::uint8_t {
	GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE, PATCH, UNKNOWN
};

inline HTTP_METHOD http_method(std::string_view method) {
	if (method == "GET") return HTTP_METHOD::GET;
	if (method == "HEAD") return HTTP_METHOD::HEAD;
	if (method == "POST") return HTTP_METHOD::POST;
	if (method == "PUT") return HTTP_METHOD::PUT;
	if (method == "DELETE") return HTTP_METHOD::DELETE;
	if (method == "CONNECT") return HTTP_METHOD::CONNECT;
	if (method == "OPTIONS") return HTTP_METHOD::OPTIONS;
	if (method == "TRACE") return HTTP_METHOD::TRACE;
	if (method == "PATCH") return HTTP_METHOD::PATCH;
	return HTTP_METHOD::UNKNOWN;
}

struct HttpResponse {
	std::string version = "HTTP/1.1";
	std::uint16_t statusCode = 200;
	std::string reason = "OK";
	std::unordered_map<std::string, std::string> headers{
		{"Access-Control-Allow-Origin", "*"},
		{"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
		{"Access-Control-Allow-Headers", "Content-Type"}
	};
	anime::protocol::ByteVector body;
};

} // namespace anime::net::http
