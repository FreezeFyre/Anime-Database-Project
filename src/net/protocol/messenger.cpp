#include "net/protocol/messenger.hpp"
#include "protocol/entry_codec.hpp"



namespace anime::protocol {

AppRequest WebSocketCodec::decode(const ByteVector& byteStream) {
	ByteReader decoder(byteStream);

	uint32_t encodeVersion = decoder.read<std::uint32_t>();
	if (encodeVersion != ENCODE_VERSION) {
		throw std::exception(" Unsupported encode version!");
	}
	COM_TYPE comType = static_cast<COM_TYPE>(decoder.read<std::uint8_t>());
	DOMAIN_TYPE domain = static_cast<DOMAIN_TYPE>(decoder.read<std::uint8_t>());
	ACTION_TYPE action = static_cast<ACTION_TYPE>(decoder.read<std::uint8_t>());
	uint32_t id = decoder.read<std::uint32_t>();
	uint32_t payloadSize = decoder.read<std::uint32_t>();

	ByteVector payload;
	payload.reserve(payloadSize);
	for (uint32_t i = 0; i < payloadSize; ++i) {
		payload.push_back(decoder.read<std::uint8_t>());
	}

	switch (domain) {
		case DOMAIN_TYPE::ENTRY: {
			EntryWebSocketCodec entryCodec;
			EntryRequest entryReq = entryCodec.decode(action, payload);
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
	writer.write(static_cast<std::uint32_t>(ENCODE_VERSION));
	writer.write(static_cast<std::uint8_t>(COM_TYPE::RESP));
	writer.write(static_cast<std::uint8_t>(request.domain));
	writer.write(static_cast<std::uint8_t>(request.action));
	writer.write(response.id);
	writer.write(anime::storage::helpers::checkedConvert<std::uint32_t>(payload.size()));
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

} // namespace anime::protocol
