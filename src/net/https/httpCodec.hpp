#pragma once

#include <unordered_map>
#include <string_view>

#include "../../databases/auth/auth_service.hpp"
#include "../routing/router.hpp"
#include "../protocol/data.hpp"
#include "../../core.hpp"


inline std::string HTTP_VERSION = "HTTP/1.1";

enum class HTTP_METHOD : uint8_t {
	GET, // No HTTP Body 
	HEAD, // Same as Get BUT NO Response Bpdy
	POST, // Gives Data To Reciever(Backend)
	PUT, // Replaces Something
	DELETE, // Deletes Something
	CONNECT, // Connects to specified server
	OPTIONS, // target communication options
	TRACE, // Message loopback along target path
	PATCH, // Modifies resource partially
	UNKNOWN
};
inline HTTP_METHOD http_method(std::string method) {
	if (method == "GET") {
		return HTTP_METHOD::GET;
	} else if (method == "HEAD") {
		return HTTP_METHOD::HEAD;
	} else if (method == "POST") {
		return HTTP_METHOD::POST;
	} else if (method == "PUT") {
		return HTTP_METHOD::PUT;
	} else if (method == "DELETE") {
		return HTTP_METHOD::DELETE;
	} else if (method == "CONNECT") {
		return HTTP_METHOD::CONNECT;
	} else if (method == "OPTIONS") {
		return HTTP_METHOD::OPTIONS;
	} else if (method == "TRACE") {
		return HTTP_METHOD::TRACE;
	} else if (method == "PATCH") {
		return HTTP_METHOD::PATCH;
	}
	return HTTP_METHOD::UNKNOWN;
}

struct HttpRequest {
	std::string method;
	std::string target;
	std::string version;

	std::unordered_map<std::string, std::string> headers;
	ByteVector body;
};


class HttpCodec {
	public:
		HttpRequest decode(ByteVector& bytes);
		ByteVector encode(const HttpResponse& response);
};

class HttpAppCodec {
	public:
		AppRequest decode(const HttpRequest& request, Context& context, Session& session);
		HttpResponse encode(AppResponse& response);
	private:
		std::vector<std::string_view> getTargetPath(const std::string& target) const;
		std::string_view getTargetQuery(const std::string& target) const;

		HttpResponse encodeHandler(const EntryResponse& response);
		HttpResponse encodeHandler(const AuthResponse& response);
};
