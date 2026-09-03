#include "net/transport/net_service.hpp"

#include <cstring>

namespace anime::net {

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
	if (!webSocketHandshake(ClientSocket)) {
		closeSocket(ClientSocket);
		return;
	}
	WebSocketCodec codec;
	Router router;
	Session session;


	do {
		WebSocketFrame frame = readWebSocketFrame(ClientSocket);

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

		AppRequest req = codec.decode(frame.payload, context, session);
		AppResponse resp = router.route(req, context, session);
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

	closeSocket(ClientSocket);
}

void NetService::httpHandler(SocketHandle ClientSocket) {
	Session session;
	Router router;
	HttpCodec httpCodec;
	HttpAppCodec appCodec;

	char recvbuf[DEFAULT_BUFLEN];
	int iResult, iSendResult;
	int recvbuflen = DEFAULT_BUFLEN;
	

	iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
	if (iResult > 0) {
		ByteVector bytes(reinterpret_cast<uint8_t*>(recvbuf), reinterpret_cast<uint8_t*>(recvbuf) + iResult);

		AppRequest req = appCodec.decode(httpCodec.decode(bytes), context, session);
		AppResponse resp = router.route(req, context, session);
		ByteVector encResp = httpCodec.encode(appCodec.encode(resp));
		iSendResult = send(ClientSocket, reinterpret_cast<const char*>(encResp.data()), static_cast<int>(encResp.size()), 0);
		if (iSendResult == SOCKET_ERROR) {
			printf("send failed: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return;
		}
		printf("Bytes sent: %d\n", iSendResult);
	} else if (iResult == 0)
		printf("Connection closing...\n");
	else {
		printf("recv failed: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return;
	}

}

} // namespace anime::net
