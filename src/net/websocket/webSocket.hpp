#pragma once

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <cctype>
#include <algorithm>
#include <limits>
#include <variant>
#include <unordered_map>
#include <array>
#include <queue>
#include <typeinfo>
#include <cstdio>

#include "net/transport/socket_compat.hpp"
#include "net/websocket/sha1.hpp"
#include "net/websocket/base64.hpp"

namespace anime::net::websocket {

inline constexpr uint8_t WEB_SOCKET_OPCODE_CONTINUATION = 0x0;
inline constexpr uint8_t WEB_SOCKET_OPCODE_TEXT = 0x1;
inline constexpr uint8_t WEB_SOCKET_OPCODE_BINARY = 0x2;
inline constexpr uint8_t WEB_SOCKET_OPCODE_CLOSE = 0x8;
inline constexpr uint8_t WEB_SOCKET_OPCODE_PING = 0x9;
inline constexpr uint8_t WEB_SOCKET_OPCODE_PONG = 0xA;

inline const std::string WEB_SOCKET_MAGIC_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";


struct WebSocketFrame {
    bool fin;
    uint8_t opcode;
    std::vector<uint8_t> payload;
};

bool webSocketHandshake(SocketHandle client);
WebSocketFrame readWebSocketFrame(SocketHandle client);

bool sendWebSocketFrame(SocketHandle client,uint8_t opcode,const std::vector<uint8_t>& payload);

std::string httpHeader(SocketHandle client);

std::string getHttpValue(const std::string& header, const std::string& key);

std::string webSockAcceptKey(const std::string& key);

} // namespace anime::net::websocket
