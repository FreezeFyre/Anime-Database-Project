#include "net/https/httpClient.hpp"

namespace anime::net::http {

HttpClientResponse HttpClient::send(const HttpClientRequest& request) {
	HttpClientResponse response;

	CURL* curl = curl_easy_init();
	if (!curl) {
		response.error = "curl_easy_init failed";
		return response;
	}

	curl_slist* headers = nullptr;

	for (const auto& header : request.headers) {
		headers = curl_slist_append(headers, header.c_str());
	}

	curl_easy_setopt(curl, CURLOPT_URL, request.url.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.body);

	if (request.method == "POST") {
		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, reinterpret_cast<const char*>(request.body.data()));
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(request.body.size()));
	}

	CURLcode code = curl_easy_perform(curl);

	if (code != CURLE_OK) {
		response.error = curl_easy_strerror(code);
	}

	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.statusCode);

	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);

	return response;
}

} // namespace anime::net::http
