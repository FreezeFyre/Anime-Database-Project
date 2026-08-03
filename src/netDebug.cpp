#include "net/https/httpCodec.hpp"

int main() {
	HttpCodec codec;

	constexpr std::string_view HTTP_REQUEST_VIEW = 
    "GET /index.html HTTP/1.1\r\n"
    "Host: ://example.com\r\n"
    "User-Agent: Mozilla/5.0\r\n"
    "Accept: text/html\r\n"
    "Connection: close\r\n"
    "\r\n";

	ByteVector request(HTTP_REQUEST_VIEW.begin(), HTTP_REQUEST_VIEW.end());

	auto message = codec.decode(request);

	_DEBUG;

	return 0;
}
