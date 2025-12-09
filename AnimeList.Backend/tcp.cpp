//
//  tcp.cpp
//  Anime List
//
//  Created by Luke Raykovitz on 11/23/25.
//

#include "tcp.hpp"

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>


int initializeListenSOCKET(int port) {
    // Create Socket (IPv4,TCP, 0)
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return -1;
    }
    
    // Allow reuse of the address after the program exits
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        // Not fatal, but we could close and return -1 if you want to be strict
    }
    
    // IP Address Structure
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1
    addr.sin_port = htons(port);
    
    // Bind Port to Address
    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return -1;
    }
    
    // Set Socket to listen
    if (listen(server_fd, 16) < 0) {
        perror("listen");
        close(server_fd);
        return -1;
    }
    
    std::cout << "Listening on 127.0.0.1:" << port << std::endl;
    return server_fd;
}


