#include "net_service.hpp"

#include <cstring>

NetService::NetService(Context& context)
	: context(context) {}




void NetService::tcpServer(uint16_t port, void (NetService::* handler)(SocketHandle)) {
	running = true;
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


void NetService::start() {
	std::thread wsThread(&NetService::tcpServer, this, INTERNAL_WS_PORT, &NetService::websocketHandler);
	wsThread.detach();
	std::thread httpThread(&NetService::tcpServer, this, INTERNAL_HTTP_PORT, &NetService::httpHandler);
	httpThread.detach();
}


void NetService::clientHandler(SocketHandle ClientSocket) {
	if (!webSocketHandshake(ClientSocket)) {
		closeSocket(ClientSocket);
		return;
	}
	Session session;
	WebSocketCodec codec;
	Router router;


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
