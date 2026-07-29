#include "messenger.hpp"

Message MessageCodec::construct(COM_TYPE comType, DOMAIN_TYPE domain, ACTION_TYPE action, uint32_t id, uint32_t payloadSize, const std::vector<uint8_t>& payload) {
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


AppRequest MessageCodec::decode(std::vector<uint8_t>& byteStream, Context& context, Session& session) {
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
			return AppRequest{comType, domain, action, id, EntryRequest{std::move(entryReq)}};
		}
		default:
			throw std::runtime_error("Unknown WebSocket domain.");
	}
}

Update MessageCodec::decodeUpdate(ByteReader& reader) {
	Update r;

	r.id = reader.U32();
	r.operation = static_cast<UpdateOp>(reader.U8());
	r.fieldIndex = reader.U8();
	r.type = static_cast<dataType>(reader.U8());
	switch (r.type) {
		case dataType::U8: {
			r.value = reader.U8();
			break;
		}
		case dataType::U16: {
			r.value = reader.U16();
			break;
		}
		case dataType::U32: {
			r.value = reader.U32();
			break;
		}
		case dataType::STR: {
			r.value = reader.STR();
			break;
		}
		case dataType::VECU8: {
			r.value = reader.VECU8();
			break;
		}
		case dataType::VECU16: {
			r.value = reader.VECU16();
			break;
		}
		default:
			throw std::runtime_error("Unknown update value data type.");
	}

	return r;
}
