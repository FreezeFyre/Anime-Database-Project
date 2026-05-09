#include "net_service.hpp"


void NetService::start(uint16_t port) {
	running = true;
	WSADATA wsaData;
	int iResult;
	struct addrinfo* result = NULL;
	struct addrinfo hints;
	

	// Winsock Initilization
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET; // IPv4 Switch to AF_INET6 or AF_UNSPEC in future
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
	}


	SOCKET ListenSocket = INVALID_SOCKET;

	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (ListenSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return;
	}


	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}
	freeaddrinfo(result);

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}

	// Rebuild to use multithreading and allow multiple clients
	while (running) {
		SOCKET ClientSocket = INVALID_SOCKET;

		ClientSocket = accept(ListenSocket, NULL, NULL);

		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return;
		}

		clientHandler(ClientSocket);

	}

	// Socket closing should be handled by stop function and errors
	closesocket(ListenSocket);
	WSACleanup(); // can be passed to stop function
	return;
}


void NetService::clientHandler(SOCKET ClientSocket) {
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

			RouteResult res = router.route(reqbuf.message);
			std::vector<uint8_t> encRes = codec.encode(res.message);
			
			iSendResult = send(ClientSocket, reinterpret_cast<const char*>(encRes.data()), encRes.size(), 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				return;
			}
			printf("Bytes sent: %d\n", iSendResult);

			if (res.closeConnection) {
				iResult = shutdown(ClientSocket, SD_SEND);
				if (iResult == SOCKET_ERROR) {
					printf("shutdown failed: %d\n", WSAGetLastError());
					closesocket(ClientSocket);
					return;
				}
				closesocket(ClientSocket);
				return;
			}
		}
		else if (iResult == 0)
			printf("Connection closing...\n");
		else {
			printf("recv failed: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			return;
		}

	} while (iResult > 0);


	// This Section would be handled by Response handler
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		return;
	}
	closesocket(ClientSocket);


}