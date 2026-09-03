#pragma once

#include <unordered_map>
#include <string_view>

#include "protocol/app_messages.hpp"
#include "net/protocol/data.hpp"
#include "net/http/http_types.hpp"


namespace anime::net::http {
using anime::protocol::ByteVector;
using anime::protocol::AuthResponse;
using anime::protocol::EntryResponse;
using anime::protocol::AppRequest;
using anime::protocol::AppResponse;

inline constexpr std::string_view APP_HTTP_VERSION = "HTTP/1.1";

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
	    using UrlForm = std::unordered_map<std::string, std::optional<std::string>>;

		AppRequest decode(const HttpRequest& request);
		HttpResponse encode(AppResponse& response);
	private:

		std::vector<std::string_view> getTargetPath(const std::string& target) const;
		std::string_view getTargetQuery(const std::string& target) const;
	    UrlForm getTargetQueries(const std::string& target) const;


		HttpResponse encodeHandler(const EntryResponse& response);
		HttpResponse encodeHandler(const AuthResponse& response);
};

} // namespace anime::net::http
