#include "messenger.hpp"



AppRequest WebSocketCodec::decode(ByteVector& byteStream, Context& context, Session& session) {
	ByteReader decoder(byteStream);

	uint32_t encodeVersion = decoder.U32();
	if (encodeVersion != ENCODE_VERSION) {
		throw std::exception(" Unsupported encode version!");
	}
	COM_TYPE comType = static_cast<COM_TYPE>(decoder.U8());
	DOMAIN_TYPE domain = static_cast<DOMAIN_TYPE>(decoder.U8());
	ACTION_TYPE action = static_cast<ACTION_TYPE>(decoder.U8());
	uint32_t id = decoder.U32();
	uint32_t payloadSize = decoder.U32();

	ByteVector payload;
	payload.reserve(payloadSize);
	for (uint32_t i = 0; i < payloadSize; ++i) {
		payload.push_back(decoder.U8());
	}

	switch (domain) {
		case DOMAIN_TYPE::ENTRY: {
			EntryWebSocketCodec entryCodec;
			EntryRequest entryReq = entryCodec.decode(action, id, payload, context, session);
			return AppRequest{comType, domain, action, id, std::move(entryReq)};
		}
		default:
			throw std::runtime_error("Unknown WebSocket domain.");
	}
}
ByteVector WebSocketCodec::encode(const AppRequest& request, const AppResponse& response) {
	// Encode Version
	// Com type
	// Domain can be extracted based of the decodeHandler Used
	// Action can be passed forwards from domain level?
	// id can be grabbed from response.id
	// payload size
	// payload

	ByteVector payload = std::visit([&](const auto& response) { return encodeHandler(response); }, response.payload);

	ByteWriter writer;
	writer.U32(ENCODE_VERSION);
	writer.U8(static_cast<uint8_t>(COM_TYPE::RESP));
	writer.U8(static_cast<uint8_t>(request.domain));
	writer.U8(static_cast<uint8_t>(request.action));
	writer.U32(response.id);
	writer.U32(payload.size());
	writer.buffer.insert(writer.buffer.end(), payload.begin(), payload.end());

	return writer.buffer;
}

ByteVector WebSocketCodec::encodeHandler(const EntryResponse& response) {
	EntryWebSocketCodec codec;
	return codec.encode(response);
}
ByteVector WebSocketCodec::encodeHandler(const AuthResponse& response) {
	throw std::runtime_error("[WebSocketCodec][ERROR] Auth not supported on web-socket.");
}
