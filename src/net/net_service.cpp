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
	int iResult;

	char recvbuf[DEFAULT_BUFLEN];
	int iSendResult;
	int recvbuflen = DEFAULT_BUFLEN;

	Request reqbuf = { ClientSocket, Message() };

	// Receive until the peer shuts down the connection
	do {
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);

		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);

			std::vector<uint8_t> bytes;
			bytes.reserve(iResult);
			for (int i = 0; i < iResult; i++) {
				bytes.push_back(static_cast<uint8_t>(recvbuf[i]));
			} // Convert the received bytes into a vector for proper handling

			try {
				reqbuf.message = codec.decode(bytes);
			}
			catch (const std::exception& e) {
				printf("Failed to decode message: %s\n", e.what());
				continue; // Skip processing this message and wait for the next one
			}

			RouteResult res = router.route(reqbuf.message, context);
			std::vector<uint8_t> encRes = codec.encode(res.message);
			
			iSendResult = send(ClientSocket, reinterpret_cast<const char*>(encRes.data()), encRes.size(), 0);
			if (iSendResult == SocketError) {
				printf("send failed: %d\n", lastSocketError());
				closeSocket(ClientSocket);
				return;
			}
			printf("Bytes sent: %d\n", iSendResult);

			if (res.closeConnection) {
				iResult = shutdown(ClientSocket, ShutdownSend);
				if (iResult == SocketError) {
					printf("shutdown failed: %d\n", lastSocketError());
					closeSocket(ClientSocket);
					return;
				}
				closeSocket(ClientSocket);
				return;
			}
		}
		else if (iResult == 0)
			printf("Connection closing...\n");
		else {
			printf("recv failed: %d\n", lastSocketError());
			closeSocket(ClientSocket);
			return;
		}

	} while (iResult > 0);


	// This Section would be handled by Response handler
	iResult = shutdown(ClientSocket, ShutdownSend);
	if (iResult == SocketError) {
		printf("shutdown failed: %d\n", lastSocketError());
		closeSocket(ClientSocket);
		return;
	}
	closeSocket(ClientSocket);


}
