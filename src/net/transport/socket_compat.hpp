#pragma once

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <ws2tcpip.h>

using SocketHandle = SOCKET;

constexpr SocketHandle InvalidSocket = INVALID_SOCKET;
constexpr int SocketError = SOCKET_ERROR;
constexpr int ShutdownSend = SD_SEND;

inline bool initSockets() {
	WSADATA wsaData;
	return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
}

inline void cleanupSockets() {
	WSACleanup();
}

inline int closeSocket(SocketHandle socket) {
	return closesocket(socket);
}

inline int lastSocketError() {
	return WSAGetLastError();
}

#else

#include <cerrno>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

using SocketHandle = int;

constexpr SocketHandle InvalidSocket = -1;
constexpr int SocketError = -1;
constexpr int ShutdownSend = SHUT_WR;

inline bool initSockets() {
	return true;
}

inline void cleanupSockets() {
}

inline int closeSocket(SocketHandle socket) {
	return close(socket);
}

inline int lastSocketError() {
	return errno;
}

#endif
