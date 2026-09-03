#include "protocol/entry_codec.hpp"
#include "domain/entry_schema.hpp"

namespace anime::protocol {
using namespace anime::domain;
using namespace anime::schema;
using namespace anime::storage;
using anime::filtering::filterProcessor;

namespace {
Update decodeUpdate(ByteReader& reader) {
	Update r;

	r.id = reader.U32();
	r.operation = static_cast<UPDATE_OPERATION>(reader.U8());
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

} // namespace

EntryRequest EntryWebSocketCodec::decode(ACTION_TYPE action, const ByteVector& payload) {
	ByteReader reader{ payload };
	std::vector<uint8_t> buffer;

	EntryRequest req;

	switch (action) {
		case ACTION_TYPE::ACT_GET:
		{
			EntryGetRequest getReq;
			getReq.ids = reader.VECU32();
			req = getReq;

			return req;
			break;
		}
		case ACTION_TYPE::ACT_UPDATE: {
			EntryUpdateRequest updateReq;

			size_t updateCount = reader.U32();
			updateReq.updates.reserve(updateCount);

			for (size_t i = 0; i < updateCount; i++) {
				Update update = decodeUpdate(reader);
				updateReq.updates.push_back(translateUpdate(update));
			}
			req = updateReq;

			return req;
			break;
		}
		case ACTION_TYPE::ACT_CREATE: {
			EntryCreateRequest createReq;

			// Check and validate schema
			schemaData schema = readSchema(reader);
			std::vector<size_t> fieldIndices;
			fieldIndices = anime::domain::validateEntrySchema(schema);

			// Construct Entries Vector
			size_t recordCount = reader.U32();
			createReq.entries.reserve(recordCount);
			for (size_t i = 0; i < recordCount; i++) {
				createReq.entries.emplace_back(readEntry(reader, fieldIndices));
			}

			req = createReq;
			return req;
			break;
		}
		case ACTION_TYPE::ACT_DELETE: {
			EntryDeleteRequest deleteReq;

			deleteReq.ids = reader.VECU32();
			req = deleteReq;

			return req;
			break;
		}
		case ACTION_TYPE::ACT_SEARCH: {
			EntrySearchRequest searchReq;

			searchReq.searchTerm = reader.STR();
			req= searchReq;

			return req;
			break;
		}
		case ACTION_TYPE::ACT_FILTER: {
			filterProcessor processor;
			EntryFilterRequest filterReq;

			filterReq.filters = processor.decode(reader);
			req = filterReq;

			return req;
			break;
		}
		default:
			std::cerr << "Unknown action type: " << static_cast<int>(action) << "\n";
			throw std::runtime_error("Unknown entry action type.");
			break;
	}
}
ByteVector EntryWebSocketCodec::encode(const EntryResponse& response) {
	return std::visit([&](const auto& resp) { return encodeResponse(resp); }, response.payload);
}

Entry EntryWebSocketCodec::readEntry(ByteReader& reader, std::vector<size_t>& fieldIndices) {
	Entry result;

	for (size_t i = 0; i < fieldIndices.size(); i++) {
		switch (entryMetadata[fieldIndices[i]].info.type) {
		case schemaField::dataType::U8:
			result.*std::get<intPTR>(entryMetadata[fieldIndices[i]].ptr) = reader.U8();
			break;
		case schemaField::dataType::U16:
			result.*std::get<intPTR>(entryMetadata[fieldIndices[i]].ptr) = reader.U16();
			break;
		case schemaField::dataType::U32:
			result.*std::get<intPTR>(entryMetadata[fieldIndices[i]].ptr) = reader.U32();
			break;
		case schemaField::dataType::STR:
			result.*std::get<strPTR>(entryMetadata[fieldIndices[i]].ptr) = reader.STR();
			break;
		case schemaField::dataType::VECU8:
			result.*std::get<vecPTR>(entryMetadata[fieldIndices[i]].ptr) = reader.VECU8();
			break;
		case schemaField::dataType::VECU16:
			result.*std::get<vecPTR>(entryMetadata[fieldIndices[i]].ptr) = reader.VECU16();
			break;
		default:
			throw std::runtime_error("Unknown data type in schema for field: " + entryMetadata[fieldIndices[i]].info.name);

		}
	}
	return result;
}
schemaData EntryWebSocketCodec::readSchema(ByteReader& reader) {
	schemaData result;
	schemaField buffer;

	size_t fieldCount = reader.U32();

	for (size_t i = 0; i < fieldCount; i++) {
		buffer.name = reader.STR();
		buffer.type = static_cast<schemaField::dataType>(reader.U8());
		buffer.mode = static_cast<schemaField::filterMode>(reader.U8());
		result.push_back(buffer);
	}

	return result;
}
sectionHeader EntryWebSocketCodec::readSection(ByteReader& reader) {
	sectionHeader header;

	header.type = static_cast<sectionType>(reader.U8());
	header.title = reader.STR();
	header.recordCount = reader.U32();
	header.fieldCount = reader.U32();
	header.hasSchema = static_cast<bool>(reader.U8());

	return header;
}
EntryUpdate EntryWebSocketCodec::translateUpdate(const Update& update) {
	EntryUpdate result;
	result.id = update.id;
	result.op = update.operation;
	result.fieldIndex = update.fieldIndex;

	switch (update.type) {
		case dataType::U8:
			result.value = static_cast<int>(std::get<uint8_t>(update.value));
			break;
		case dataType::U16:
			result.value = static_cast<int>(std::get<uint16_t>(update.value));
			break;
		case dataType::U32: {
			uint32_t raw = std::get<uint32_t>(update.value);
			if (raw > static_cast<uint32_t>(std::numeric_limits<int>::max())) {
				throw std::runtime_error("Update value exceeds entry integer range.");
			}
			result.value = static_cast<int>(raw);
			break;
		}
		case dataType::STR:
			result.value = std::get<std::string>(update.value);
			break;
		case dataType::VECU8:
		case dataType::VECU16:
			result.value = std::get<std::vector<int>>(update.value);
			break;
		default:
			throw std::runtime_error("Unknown update value data type.");
	}

	return result;
}

void EntryWebSocketCodec::encodeSection(ByteWriter& encoder, const sectionHeader& section) {
	encoder.U8(static_cast<uint8_t>(section.type));
	encoder.STR(section.title);
	encoder.U32(section.recordCount);
	encoder.U32(section.fieldCount);
	encoder.U8(static_cast<uint8_t>(section.hasSchema));
}
void EntryWebSocketCodec::encodeSchema(ByteWriter& encoder, const schemaData& schema) {
	encoder.U32(schema.size());

	for (const auto& field : schema) {
		encoder.STR(field.name);
		encoder.U8(static_cast<uint8_t>(field.type));
		encoder.U8(static_cast<uint8_t>(field.mode));
	}
}
void EntryWebSocketCodec::encodeEntry(ByteWriter& encoder, const Entry& entry) {
	for (size_t i = 0; i < entryMetadata.size(); i++) {
		switch (entryMetadata[i].info.type) {
		case schemaField::dataType::U8:
			encoder.U8(entry.*std::get<intPTR>(entryMetadata[i].ptr));
			break;
		case schemaField::dataType::U16:
			encoder.U16(entry.*std::get<intPTR>(entryMetadata[i].ptr));
			break;
		case schemaField::dataType::U32:
			encoder.U32(entry.*std::get<intPTR>(entryMetadata[i].ptr));
			break;
		case schemaField::dataType::STR:
			encoder.STR(entry.*std::get<strPTR>(entryMetadata[i].ptr));
			break;
		case schemaField::dataType::VECU8:
			encoder.VECU8(entry.*std::get<vecPTR>(entryMetadata[i].ptr));
			break;
		case schemaField::dataType::VECU16:
			encoder.VECU16(entry.*std::get<vecPTR>(entryMetadata[i].ptr));
			break;
		}
	}
}

ByteVector EntryWebSocketCodec::encodeResponse(const EntryGetResponse& response) {
	ByteWriter encoder;

	encodeSchema(encoder, response.schema);
	encoder.U32(response.entries.size());
	for (const Entry& entry : response.entries) {
		encodeEntry(encoder, entry);
	}

	return encoder.buffer;
}
ByteVector EntryWebSocketCodec::encodeResponse(const EntryIdsResponse& response) {
	ByteWriter encoder;

	encoder.VECU32(response.ids);

	return encoder.buffer;
}
ByteVector EntryWebSocketCodec::encodeResponse(const EntrySuccessResponse& response) {
	ByteWriter encoder;

	encoder.STR(response.message);

	return encoder.buffer;
}
ByteVector EntryWebSocketCodec::encodeResponse(const EntryErrorResponse& response) {
	ByteWriter encoder;

	encoder.STR(response.message);

	return encoder.buffer;
}



} // namespace anime::protocol
