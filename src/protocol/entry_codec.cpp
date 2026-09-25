#include "protocol/entry_codec.hpp"
#include "domain/entry_schema.hpp"

#include <type_traits>


namespace anime::protocol {
using namespace anime::domain;
using namespace anime::schema;
using namespace anime::storage;
using anime::filtering::filterProcessor;

namespace {
Update decodeUpdate(ByteReader& reader) {
	Update r;

	r.id = reader.read<std::uint32_t>();
	r.operation = static_cast<UPDATE_OPERATION>(reader.read<std::uint8_t>());
	r.fieldIndex = reader.read<std::uint8_t>();
	r.type = static_cast<dataType>(reader.read<std::uint8_t>());
	switch (r.type) {
		case dataType::U8: {
			r.value = reader.read<std::uint8_t>();
			break;
		}
		case dataType::U16: {
			r.value = reader.read<std::uint16_t>();
			break;
		}
		case dataType::U32: {
			r.value = reader.read<std::uint32_t>();
			break;
		}
		case dataType::STR: {
			r.value = reader.read<std::string>();
			break;
		}
		case dataType::VECU8: {
			r.value = reader.read<std::vector<std::uint8_t>>();
			break;
		}
		case dataType::VECU16: {
			r.value = reader.read<std::vector<std::uint16_t>>();
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
			getReq.ids = reader.read <std::vector<std::uint32_t>>();
			req = getReq;

			return req;
			break;
		}
		case ACTION_TYPE::ACT_UPDATE: {
			EntryUpdateRequest updateReq;

			size_t updateCount = reader.read<std::uint32_t>();
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
			size_t recordCount = reader.read<std::uint32_t>();
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

			deleteReq.ids = reader.read<std::vector<std::uint32_t>>();
			req = deleteReq;

			return req;
			break;
		}
		case ACTION_TYPE::ACT_SEARCH: {
			EntrySearchRequest searchReq;

			searchReq.searchTerm = reader.read<std::string>();
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
		std::visit([&](const auto& memberPtr) {
			using FieldType = std::remove_cvref_t<decltype(result.*memberPtr)>;
			result.*memberPtr = reader.read<FieldType>();
		}, entryMetadata[fieldIndices[i]].ptr);
	}
	return result;
}
schemaData EntryWebSocketCodec::readSchema(ByteReader& reader) {
	schemaData result;
	schemaField buffer;

	size_t fieldCount = reader.read<std::uint32_t>();

	for (size_t i = 0; i < fieldCount; i++) {
		buffer.name = reader.read<std::string>();
		buffer.type = static_cast<schemaField::dataType>(reader.read<std::uint8_t>());
		buffer.mode = static_cast<schemaField::filterMode>(reader.read<std::uint8_t>());
		result.push_back(buffer);
	}

	return result;
}
sectionHeader EntryWebSocketCodec::readSection(ByteReader& reader) {
	sectionHeader header;

	header.type = static_cast<sectionType>(reader.read<std::uint8_t>());
	header.title = reader.read<std::string>();
	header.recordCount = reader.read<std::uint32_t>();
	header.fieldCount = reader.read<std::uint32_t>();
	header.hasSchema = static_cast<bool>(reader.read<std::uint8_t>());

	return header;
}
EntryUpdate EntryWebSocketCodec::translateUpdate(const Update& update) {
	EntryUpdate result;
	result.id = update.id;
	result.op = update.operation;
	result.fieldIndex = update.fieldIndex;

	std::visit([&](const auto& value) {
		using FieldType = std::remove_cvref_t<decltype(result.value)>;
		using UpdateType = std::remove_cvref_t<decltype(result.value)>; // probably want to swap to use non-exact type matching, idk
		if constexpr (std::same_as<FieldType, UpdateType>) {
			result.value = update.value;
		} else {
			throw std::runtime_error("Update type must be the same as the target field. ");
		}
	}, update.value);

	return result;
}

void EntryWebSocketCodec::encodeSection(ByteWriter& encoder, const sectionHeader& section) {
	encoder.write(static_cast<uint8_t>(section.type));
	encoder.write(section.title);
	encoder.write(section.recordCount);
	encoder.write(section.fieldCount);
	encoder.write(static_cast<uint8_t>(section.hasSchema));
}
void EntryWebSocketCodec::encodeSchema(ByteWriter& encoder, const schemaData& schema) {
	encoder.write(static_cast<std::uint32_t>(schema.size()));

	for (const auto& field : schema) {
		encoder.write(field.name);
		encoder.write(static_cast<uint8_t>(field.type));
		encoder.write(static_cast<uint8_t>(field.mode));
	}
}
void EntryWebSocketCodec::encodeEntry(ByteWriter& encoder, const Entry& entry) {
	for (size_t i = 0; i < entryMetadata.size(); i++) {
		std::visit([&](const auto& memberPtr) {
			encoder.write(entry.*memberPtr);
		}, entryMetadata[i].ptr);
	}
}

ByteVector EntryWebSocketCodec::encodeResponse(const EntryGetResponse& response) {
	ByteWriter encoder;

	encodeSchema(encoder, response.schema);
	encoder.write(static_cast<uint32_t>(response.entries.size()));
	for (const Entry& entry : response.entries) {
		encodeEntry(encoder, entry);
	}

	return encoder.buffer;
}
ByteVector EntryWebSocketCodec::encodeResponse(const EntryIdsResponse& response) {
	ByteWriter encoder;

	encoder.write(response.ids);

	return encoder.buffer;
}
ByteVector EntryWebSocketCodec::encodeResponse(const EntrySuccessResponse& response) {
	ByteWriter encoder;

	encoder.write(response.message);

	return encoder.buffer;
}
ByteVector EntryWebSocketCodec::encodeResponse(const EntryErrorResponse& response) {
	ByteWriter encoder;

	encoder.write(response.message);

	return encoder.buffer;
}



} // namespace anime::protocol
