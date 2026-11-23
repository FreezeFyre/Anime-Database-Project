#include "net/transport/net_service.hpp"

#include <cstring>

namespace anime::net {
using namespace anime::protocol;
using namespace anime::app;
using namespace anime::net::http;
using namespace anime::net::websocket;

NetService::NetService(Context& context)
	: context(context) {}



void NetService::start() {
	if (running) return;

	running = true;
	curl_global_init(CURL_GLOBAL_DEFAULT);

	listenerThreads.emplace_back(&NetService::tcpServer, this, INTERNAL_WS_PORT, &NetService::websocketHandler);
	listenerThreads.emplace_back(&NetService::tcpServer, this, INTERNAL_HTTP_PORT, &NetService::httpHandler);
}

void NetService::run() {
	start();

	for (auto& thread : listenerThreads) {
		if (thread.joinable()) {
			thread.join();
		}
	}
}

void NetService::stop() {
	curl_global_cleanup();
	running = false;
}



void NetService::tcpServer(uint16_t port, void (NetService::* handler)(SocketHandle)) {
	int iResult;
	struct addrinfo* result = NULL;
	struct addrinfo hints;
	

	if (!initSockets()) {
		printf("socket initialization failed: %d\n", lastSocketError());
		return;
	}

	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // IPv4 Switch to AF_INET6 or AF_UNSPEC in future
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		cleanupSockets();
		return;
	}


	SocketHandle ListenSocket = InvalidSocket;

	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (ListenSocket == InvalidSocket) {
		printf("Error at socket(): %d\n", lastSocketError());
		freeaddrinfo(result);
		cleanupSockets();
		return;
	}


	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SocketError) {
		printf("bind failed with error: %d\n", lastSocketError());
		freeaddrinfo(result);
		closeSocket(ListenSocket);
		cleanupSockets();
		return;
	}
	freeaddrinfo(result);

	if (listen(ListenSocket, SOMAXCONN) == SocketError) {
		printf("Listen failed with error: %d\n", lastSocketError());
		closeSocket(ListenSocket);
		cleanupSockets();
		return;
	}

	while (running) {
		SocketHandle ClientSocket = InvalidSocket;

		ClientSocket = accept(ListenSocket, NULL, NULL);

		if (ClientSocket == InvalidSocket) {
			printf("accept failed: %d\n", lastSocketError());
			closeSocket(ListenSocket);
			cleanupSockets();
			return;
		}


		try {
			std::thread([this, handler, ClientSocket] { (this->*handler)(ClientSocket); }).detach();

			auto now = std::chrono::system_clock::now();
			std::clog << now << "\t" << "[INFO] client connected\n";
		} catch (std::system_error const &e) {
			std::cerr << e.what() << std::endl;
		}
	}
}




void NetService::websocketHandler(SocketHandle ClientSocket) {
	std::string token;
	try {
		if (!webSocketHandshake(ClientSocket, token)) {
			closeSocket(ClientSocket);
			return;
		}
		WebSocketCodec codec;
		Router router;

		// TODO: Finish WebSocket connection handling:
		// - Reject oversized payloads before allocating memory in readWebSocketFrame().
		// - Preserve leftover handshake bytes for subsequent frame reads.
		// - Complete handshake and control-frame validation.

		do {
			auto res = readWebSocketFrame(ClientSocket);
			if (!res) {
				closeSocket(ClientSocket);
				return;
			}
			WebSocketFrame frame = *res;

			if (frame.opcode == WEB_SOCKET_OPCODE_CLOSE) {
				sendWebSocketFrame(ClientSocket, WEB_SOCKET_OPCODE_CLOSE, {});
				break;
			}

			if (frame.opcode == WEB_SOCKET_OPCODE_PING) {
				sendWebSocketFrame(ClientSocket, WEB_SOCKET_OPCODE_PONG, frame.payload);
				continue;
			}

			if (frame.opcode != WEB_SOCKET_OPCODE_BINARY) {
				std::cerr << "Unsupported WebSocket opcode: " << static_cast<int>(frame.opcode) << std::endl;
				continue;
			}

			AppRequest req;
			try {
				req = codec.decode(frame.payload);
			} catch (const std::exception& error) {
				std::cerr << "Invalid WebSocket request: " << error.what() << '\n';
				break;
			}
			AppResponse resp = router.route(req, context, token);
			ByteVector encResp = codec.encode(req, resp);

			if (!sendWebSocketFrame(ClientSocket, WEB_SOCKET_OPCODE_BINARY, encResp)) {
				printf("send failed: %d\n", lastSocketError());
				break;
			}

			if (resp.closeConnection) {
				sendWebSocketFrame(ClientSocket, WEB_SOCKET_OPCODE_CLOSE, {});
				break;
			}
		} while (running);
	} catch (const std::exception& e) {
		std::cerr << e.what();
	}
	closeSocket(ClientSocket);
}

void NetService::httpHandler(SocketHandle ClientSocket) {
	Router router;
	HttpCodec httpCodec;
	HttpAppCodec appCodec;

	try {
		std::string buffer;
		HttpRequest httpRequest = readHttpRequest(ClientSocket, buffer);

		const auto token = retrieveCookie(httpRequest.headers, "session");
		AppRequest req = appCodec.decode(httpRequest);
		AppResponse resp = router.route(req, context, token);
		ByteVector encResp = httpCodec.encode(appCodec.encode(resp));
		int totalSent = 0;
		while (totalSent < encResp.size()) {
			int sent = send(ClientSocket, reinterpret_cast<const char*>(encResp.data()+totalSent), static_cast<int>(encResp.size()-totalSent), 0);
			if (sent <= 0) {
				throw std::runtime_error("Failed to send HTTP response");
			}
			totalSent += static_cast<size_t>(sent);
		}
	} catch (const std::exception& e) {
		std::cerr << e.what();
	}
	closeSocket(ClientSocket);
}

} // namespace anime::net
