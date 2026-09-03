#pragma once

#include <unordered_map>
#include <string_view>

#include "net/routing/router.hpp"
#include "net/protocol/data.hpp"
#include "core.hpp"
#include <curl/curl.h>

#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#pragma comment(lib, "Ws2_32.lib")


#else
	#include <cerrno>
	#include <netdb.h>
	#include <sys/socket.h>
	#include <unistd.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
#endif


namespace anime::net::http {
using anime::protocol::ByteVector;
inline constexpr uint16_t DEFAULT_BUFLEN = 512;
inline constexpr uint16_t DEFAULT_PORT = 27015;


struct HttpClientRequest {
	std::string method;
	std::string url;
	std::vector<std::string> headers;
	ByteVector body;
};

struct HttpClientResponse {
	long statusCode = 0;
	ByteVector body;
	std::string error;
};


class HttpClient {
	public:
		HttpClientResponse send(const HttpClientRequest& request);

};

} // namespace anime::net::http
