#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <cstdint>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")

class TCP {
	public:
		void start(uint16_t port);

		void send();
	private:
		int socket();
};

struct request {

};

struct response {

};

struct queueItem {

};

class NetService {
	public:
		void start(uint16_t port);

		void end();

	private:
		std::vector<queueItem> queueStack;


};