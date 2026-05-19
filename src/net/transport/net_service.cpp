#include "net_service.hpp"

#include <cstring>


void NetService::start(uint16_t port) {
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

	// Rebuild to use multithreading and allow multiple clients
	while (running) {
		SocketHandle ClientSocket = InvalidSocket;

		ClientSocket = accept(ListenSocket, NULL, NULL);

		if (ClientSocket == InvalidSocket) {
			printf("accept failed: %d\n", lastSocketError());
			closeSocket(ListenSocket);
			cleanupSockets();
			return;
		}

		clientHandler(ClientSocket);

	}

	// Socket closing should be handled by stop function and errors
	closeSocket(ListenSocket);
	cleanupSockets(); // can be passed to stop function
	return;
}


void NetService::clientHandler(SocketHandle ClientSocket) {
	if (!webSocketHandshake(ClientSocket)) {
		closeSocket(ClientSocket);
		return;
	}

	Request reqbuf = { ClientSocket, Message() };

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

		try {
			reqbuf.message = codec.decode(frame.payload);
		}
		catch (const std::exception& e) {
			printf("Failed to decode message: %s\n", e.what());
			continue;
		}

		RouteResult res = router.route(reqbuf.message, context);
		std::vector<uint8_t> encRes = codec.encode(res.message);

		if (!sendWebSocketFrame(ClientSocket, WEB_SOCKET_OPCODE_BINARY, encRes)) {
			printf("send failed: %d\n", lastSocketError());
			break;
		}

		if (res.closeConnection) {
			sendWebSocketFrame(ClientSocket, WEB_SOCKET_OPCODE_CLOSE, {});
			break;
		}

	} while (running);

	closeSocket(ClientSocket);
}
