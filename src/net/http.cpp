#include "http.hpp"

Message MessageConstructor::construct(COM_TYPE comType, DOMAIN_TYPE domain, ACTION_TYPE action, uint32_t id, uint32_t payloadSize, const std::vector<uint8_t>& payload) {
	Message message;
	message.encodeVersion = ENCODE_VERSION;
	message.comType = comType;
	message.domain = domain;
	message.action = action;
	message.id = id;
	message.payloadSize = payloadSize;
	message.payload = payload;
	return message;
}

std::vector<uint8_t> MessageCodec::encode(const Message& message) {
	std::vector<uint8_t> buffer;

	ByteWriter w;

	w.U32(message.encodeVersion);
	w.U8(static_cast<uint8_t>(message.comType));
	w.U8(static_cast<uint8_t>(message.domain));
	w.U8(static_cast<uint8_t>(message.action));
	w.U32(message.id);
	w.U32(message.payloadSize);
	w.buffer.insert(w.buffer.end(), message.payload.begin(), message.payload.end());

	return w.buffer;
}


Message MessageCodec::decode(std::vector<uint8_t>& byteStream) {
	Message message;



	return message;
}