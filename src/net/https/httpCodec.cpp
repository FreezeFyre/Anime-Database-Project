#include "net/https/httpCodec.hpp"
#include "databases/auth/auth_service.hpp"

// Need to rewrite logic cleaner just trying to get the basic idea down and also need to add normalization
namespace anime::net::http {
using namespace anime::protocol;
anime::protocol::LibraryResponse;


HttpRequest HttpCodec::decode(ByteVector& bytes) {
	HttpRequest request;

	const std::string_view raw(
		reinterpret_cast<const char*>(bytes.data()),
		bytes.size()
	);

	const auto headerEnd = raw.find("\r\n\r\n");
	if (headerEnd == std::string_view::npos) {
		throw std::runtime_error("Incomplete HTTP headers.");
	}

	const auto startLineEnd = raw.find("\r\n");
	const auto startLine = raw.substr(0, startLineEnd);

	const auto firstSpace = startLine.find(' ');
	if (firstSpace == std::string_view::npos || firstSpace == 0) {
		throw std::runtime_error("Invalid HTTP request line.");
	}

	const auto secondSpace = startLine.find(' ', firstSpace + 1);
	if (secondSpace == std::string_view::npos ||
		secondSpace == firstSpace + 1 ||
		secondSpace + 1 == startLine.size() ||
		startLine.find(' ', secondSpace + 1) != std::string_view::npos) {
		throw std::runtime_error("Invalid HTTP request line.");
	}

	request.method = std::string(startLine.substr(0, firstSpace));
	request.target = std::string(
		startLine.substr(firstSpace + 1, secondSpace - firstSpace - 1)
	);
	request.version = std::string(startLine.substr(secondSpace + 1));

	if (request.version != APP_HTTP_VERSION) {
		throw std::runtime_error("Bad HTTP version: " + request.version);
	}

	const auto headersStart = startLineEnd + 2;

	request.headers = parseHeaders(
		raw.substr(headersStart, headerEnd + 2 - headersStart)
	);

	const auto bodyStart = headerEnd + 4;
	const auto body = raw.substr(bodyStart);
	request.body.assign(body.begin(), body.end());

	return request;
}
ByteVector HttpCodec::encode(const HttpResponse& response) {
	std::string header;
	
	header += response.version + " ";
	header += std::to_string(response.statusCode) + " ";
	header += response.reason + "\r\n";
	
	bool hasContentLength = false;
	
	for (const auto& [key, value] : response.headers) {
		if (key == "Content-Length") {
			hasContentLength = true;
		}
	
		header += key + ": " + value + "\r\n";
	}
	
	if (!hasContentLength) {
		header += "Content-Length: ";
		header += std::to_string(response.body.size());
		header += "\r\n";
	}
	
	header += "\r\n";
	
	ByteVector result;
	result.reserve(header.size() + response.body.size());
	
	result.insert(result.end(), header.begin(), header.end());
	result.insert(result.end(), response.body.begin(), response.body.end());
	
	return result;
}



AppRequest HttpAppCodec::decode(const HttpRequest& request) {
	std::vector<std::string_view> path = getTargetPath(request.target);
	
	std::string_view query;
	if (request.target.find("?") != std::string::npos) {
		query = getTargetQuery(request.target);
	}

	switch (DOMAIN_TYPE domain = domain_type(std::string(path[0]))) {
		case DOMAIN_TYPE::AUTH: {
			AuthHttpCodec codec;

			ACTION_TYPE action = action_type(std::string(path[1]));
			uint32_t id = 0;

			auto req = codec.decode(action, path, request.body);
			return AppRequest{COM_TYPE::REQ, domain, static_cast<std::uint8_t>(action), id, req};
		}
		default:
			throw std::runtime_error("Unkown/Unsupported domain in target: " + request.target);

	}
}
HttpResponse HttpAppCodec::encode(AppResponse& response) {
	return std::visit([&](const auto& response) { return encodeHandler(response); }, response.payload);
}

// Should modify to work with non origin form targets
std::vector<std::string_view> HttpAppCodec::getTargetPath(const std::string& target) const {
	if (target.size() == std::string::npos) {
		throw std::runtime_error("Bad Target Path: " + target + "\n");
		return std::vector<std::string_view>();
	} else if (target.size() == 1) {
		throw std::runtime_error("Empty Target Path: " + target + "\n");
		return std::vector<std::string_view>();
	}
	std::vector<std::string_view> result;
	std::string_view raw = target;

	size_t pos = raw.find("?");
	if (pos != std::string_view::npos) {
	    raw.remove_suffix(raw.size() - pos);
	}

	if (raw.find("/") == 0) {
		raw.remove_prefix(1);
	}
	while (true) {
		size_t pos = raw.find("/");
		if (pos == std::string_view::npos) break;

		result.push_back(raw.substr(0, pos));
		raw.remove_prefix(pos + 1);
	}
	result.push_back(raw);

	return result;
}
// Should modify to work with multiple query params and also parse them into key value pairs
std::string_view HttpAppCodec::getTargetQuery(const std::string& target) const {
	std::string_view raw = target;
	size_t pos = raw.find("?");
	if (pos == std::string_view::npos) {
		throw std::runtime_error("No Target Query Found \n");
	}
	raw.remove_prefix(pos);
	return {raw};
}
HttpAppCodec::UrlForm HttpAppCodec::getTargetQueries(const std::string& target) const {
	using std::string;

	UrlForm queries;
	std::string_view raw{target};

	size_t pos = raw.find("?");
	if (pos == std::string_view::npos) {
		throw std::runtime_error("No Target Query Found \n");
	}
	raw.remove_prefix(pos);

	std::vector<std::string_view> query_params;

	size_t next_pos;
	do {
		next_pos = raw.find("&");
		bool has_next = next_pos != std::string_view::npos;


		size_t val_pos = raw.find("=");
		bool has_val = val_pos != std::string_view::npos;
		if (has_val) {
			std::string key{raw.substr(0, val_pos)};
			if (has_next) {
				queries[key] = string{raw.substr(val_pos + 1, next_pos - val_pos - 1)};
			} else {
				queries[key] = string{raw.substr(val_pos + 1, raw.size() - val_pos - 1)};
			}
		}
		if (has_next) {
			queries[string{raw.substr(0, next_pos)}] = std::nullopt;
		} else {
			break;
		}
	} while (next_pos != std::string_view::npos);
	// Here would handle not having a value for a query param, still have handling in the loop to handle a single key or key/value pair before it reaches the while statement


}


HttpResponse HttpAppCodec::encodeHandler(const EntryResponse& response) {
	std::string error = "[Http][ERROR] Entries not supported on http.";
	return HttpResponse {
		std::string(APP_HTTP_VERSION),
		405,
		"Method Not Allowed",
		{{"Content-Type", "application / octet - stream "}},
		ByteVector{error.begin(), error.end()}
	};
}
HttpResponse HttpAppCodec::encodeHandler(const AuthResponse& response) {
	AuthHttpCodec codec;
	return codec.encode(response);
}

HttpResponse HttpAppCodec::encodeHandler(const LibraryResponse& response) {
	const std::string error = "Library requests require WebSocket.";
	return {std::string(APP_HTTP_VERSION),
		    405,
		    "Method Not Allowed",
		    {{"Content-Type", "text/plain"}},
		    ByteVector{error.begin(), error.end()}};
}

} // namespace anime::net::http
