#include "tcp.hpp"

void TCP::start(uint16_t port) {
	WSADATA wsaData;
	int iResult;

	// Winsock Initilization
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
	}
}