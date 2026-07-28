#include "webSocket.hpp"



bool recvAll(SocketHandle client, void* data, size_t size) {
	char* buffer = static_cast<char*>(data);
	size_t total = 0;

	while (total < size) {
		int received = recv(
			client,
			buffer + total,
			static_cast<int>(size - total),
			0
		);

		if (received <= 0) {
			return false;
		}

		total += received;
	}

	return true;
}


bool sendAll(SocketHandle client, const void* data, size_t size) {
	const char* buffer = static_cast<const char*>(data);
	size_t total = 0;

	while (total < size) {
		int sent = send(
			client,
			buffer + total,
			static_cast<int>(size - total),
			0
		);

		if (sent <= 0) {
			return false;
		}

		total += sent;
	}

	return true;
}


bool webSocketHandshake(SocketHandle client) {
	// Placeholder for handshake implementation
	std::string header = httpHeader(client);
	if (header.empty()) {
		return false;
	}

	std::string key = getHttpValue(header, "Sec-WebSocket-Key");
	if (key.empty()) {
		return false;
	}

	std::string accept = webSockAcceptKey(key);

	std::string response =
		"HTTP/1.1 101 Switching Protocols\r\n"
		"Upgrade: websocket\r\n"
		"Connection: Upgrade\r\n"
		"Sec-WebSocket-Accept: " + accept + "\r\n"
		"\r\n";

	send(client, response.c_str(), response.size(), 0);

	return true;
}

WebSocketFrame readWebSocketFrame(SocketHandle client) {
	// Placeholder for frame reading implementation
	char buffer[512];

	recvAll(client, buffer, 2);

	uint8_t b0 = buffer[0];

	bool fin = (b0 & 0x80) != 0;
	bool rsv1 = (b0 & 0x40) != 0;
	bool rsv2 = (b0 & 0x20) != 0;
	bool rsv3 = (b0 & 0x10) != 0;
	uint8_t opcode = b0 & 0x0F;

	if (!fin) {
		std::cerr << "Fragmented frames not supported" << std::endl;
		return { false, 0, {} };
	}

	if (rsv1 or rsv2 or rsv3) {
		std::cerr << "Websocket extension not supported" << std::endl;
		return { false, 0, {} };
	}

	uint8_t b1 = buffer[1];

	bool masked = (b1 & 0x80) != 0;
	uint64_t payloadLength = b1 & 0x7F;

	if (!masked) {
		std::cerr << "Unmasked frame received" << std::endl;
		return { false, 0, {} };
	}

	if (payloadLength == 126) {
		uint8_t ext[2];
		recvAll(client, ext, 2);

		payloadLength =
			(static_cast<uint64_t>(ext[0]) << 8) |
			static_cast<uint64_t>(ext[1]);
	}
	else if (payloadLength == 127) {
		uint8_t ext[8];
		recvAll(client, ext, 8);

		payloadLength = 0;
		for (int i = 0; i < 8; ++i) {
			payloadLength = (payloadLength << 8) | ext[i];
		}
	}

	uint8_t maskKey[4];
	recvAll(client, maskKey, 4);

	std::vector<uint8_t> payload(payloadLength);
	recvAll(client, payload.data(), payload.size());

	for (uint64_t i = 0; i < payloadLength; ++i) {
		payload[i] ^= maskKey[i % 4];
	}

	return WebSocketFrame{ fin, opcode, payload };
}

std::string httpHeader(SocketHandle client) {
	std::string request;
	char buffer[512];

	while (request.find("\r\n\r\n") == std::string::npos) {
		int received = recv(client, buffer, sizeof(buffer), 0);

		if (received <= 0) {
			return "";
		}

		request.append(buffer, received);
	}

	return request;
}

std::string getHttpValue(const std::string& header, const std::string& key) {
	size_t pos = header.find(key + ":");
	if (pos == std::string::npos) {
		return "";
	}
	size_t start = pos + key.length() + 2;

	while (start < header.size() && header[start] == ' ') {
		start++;
	}

	size_t end = header.find("\r\n", start);
	if (end == std::string::npos) {
		return "";
	}
	return header.substr(start, end - start);
}

std::string webSockAcceptKey(const std::string& key) {
	auto digest = sha1Bytes(key + WEB_SOCKET_MAGIC_GUID);
	return base64Encode(digest);
}


bool sendWebSocketFrame(SocketHandle client, uint8_t opcode, const std::vector<uint8_t>& payload) {
	std::vector<uint8_t> frame;
	
	frame.push_back(static_cast<uint8_t>(0x80 | opcode));
	const uint64_t payloadLength = payload.size();

	if (payloadLength <= 125) {
		frame.push_back(static_cast<uint8_t>(payloadLength));
	}
	else if (payloadLength <= 65535) {
		frame.push_back(126);
		frame.push_back((payloadLength >> 8) & 0xFF);
		frame.push_back(payloadLength & 0xFF);
	}
	else {
		frame.push_back(127);
		for (int i = 56; i >= 0; i -= 8) {
			frame.push_back(static_cast<uint8_t>((payloadLength >> i) & 0xFF));
		}
	}

	frame.insert(frame.end(), payload.begin(), payload.end());


	return sendAll(client, frame.data(), frame.size());
}