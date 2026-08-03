#include "httpCodec.hpp"

// Need to rewrite logic cleaner just trying to get the basic idea down and also need to add normalization
HttpRequest HttpCodec::decode(ByteVector& bytes) {
	HttpRequest request;

	std::string_view raw(reinterpret_cast<char*>(bytes.data()), bytes.size());

	size_t length = raw.length();
	size_t startLineEnd = raw.find("\r\n");
	std::string_view startLine = raw.substr(0,startLineEnd);
	raw.remove_prefix(startLineEnd+2);

	size_t pos = startLine.find(" ");
	request.method =  std::string(startLine.substr(0,pos));
	startLine.remove_prefix(pos+1);
	pos = startLine.find(" ");
	request.target =  std::string(startLine.substr(0,pos));
	startLine.remove_prefix(pos+1);
	request.version = std::string(startLine);
	if (request.version != HTTP_VERSION) {
		throw std::runtime_error("Bad Http Version: " + request.version + "\n");
	}

	size_t headerEnd = raw.find("\r\n\r\n");
	raw.remove_suffix(raw.length() - headerEnd);

	// raw now == headers
	size_t cursor = 0;
	while (raw.find("\r\n") != std::string_view::npos) {
		cursor = raw.find(":");
		std::string key = std::string(raw.substr(0, cursor));
		raw.remove_prefix(cursor);
		cursor = raw.find("\r\n");
		std::string value = std::string(raw.substr(1, cursor));
		raw.remove_prefix(cursor+2);

		request.headers[key] = value;
	}

	ByteVector body(bytes.begin() + headerEnd +  startLineEnd + 6, bytes.end());
	request.body = body;

	return HttpRequest();
}
ByteVector HttpCodec::encode(const HttpResponse& response) {
	ByteVector result;
	ByteWriter writer;

	writer.STR(response.version + " " + std::to_string(response.statusCode) + " " + response.reason + "\r\n");
	for (const auto& [key, value] : response.headers) {
		writer.STR(key + ": " + value + "\r\n");
	}
	writer.STR("\r\n");
	
	result.reserve(writer.buffer.size() + response.body.size());
	result.insert(result.end(), writer.buffer.begin(), writer.buffer.end());
	result.insert(result.end(), response.body.begin(), response.body.end());

	return result;
}



AppRequest HttpAppCodec::decode(const HttpRequest& request, Context& context, Session& session) {
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

			auto req = codec.decode(action, path, request.headers, request.body);
			return AppRequest{COM_TYPE::REQ, domain, action, id, req};
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

HttpResponse HttpAppCodec::encodeHandler(const EntryResponse& response) {
	std::string error = "[Http][ERROR] Entries not supported on http.";
	return HttpResponse {
		"HTTP/1.1",
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

