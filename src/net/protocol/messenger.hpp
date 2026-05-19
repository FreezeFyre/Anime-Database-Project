#include <string>
#include <unordered_map>
#include <cstdint>
#include <vector>
#include <variant>

#include "data.hpp"
#include "../../io/fileHandling.hpp"

#pragma once

#define ENCODE_VERSION 1001


enum class COM_TYPE : uint8_t {
	REQ = 1,
	RESP = 2,
	STATUS = 3,
	PING = 4
};
enum class ACTION_TYPE : uint8_t {
	ACT_GET = 1,
	ACT_UPDATE = 2,
	ACT_CREATE = 3,
	ACT_DELETE = 4,
	ACT_SEARCH = 5,
	ACT_SORT = 6,
	ACT_APPLY = 7
};
enum class DOMAIN_TYPE : uint8_t {
	ENTRY = 1,
	GROUPS = 2,
	CHARACTERS = 3,
	FILTERS = 4,
	AUTH = 10
};

enum class UpdateOp : uint8_t {
	SET_FIELD,
	CLEAR_FIELD,
	ADD_VALUE,
	REMOVE_VALUE
};

struct Update {
	uint32_t id;
	UpdateOp operation;
	uint8_t fieldIndex;
	dataType type;
	std::variant<std::monostate, uint8_t, uint16_t, uint32_t, std::string, std::vector<uint8_t>, std::vector<uint16_t>> value;
};

struct Message {
	uint32_t encodeVersion;
	COM_TYPE comType;
	DOMAIN_TYPE domain;
	ACTION_TYPE action;
	uint32_t id;
	uint32_t payloadSize;
	std::vector<uint8_t> payload;
};

/*

REQUEST FORMAT


[ENCODE_VERSION]
[COM_TYPE]
[DOMAIN]
[ACTION]
[ID]
[PAYLOAD_SIZE]
[PAYLOAD]


*/

class MessageCodec {

	/*
	What is the Encoders job?
	Encoders job is to take different variables and construct a byte vector that can be sent over network and decoded.

	What is the Decoders job?
	Decoders job is to take a byte vector and decode it into data that can be passed be used by a router to route the remainig un-decoded byte data (Payload) to the correct de-serializer and handler.
	
	*/
	public:
		std::vector<uint8_t> encode(const Message& message);

		Message decode(std::vector<uint8_t>& byteStream);

		Message construct(COM_TYPE comType, DOMAIN_TYPE domain, ACTION_TYPE action, uint32_t id, uint32_t payloadSize, const std::vector<uint8_t>& payload);

};
