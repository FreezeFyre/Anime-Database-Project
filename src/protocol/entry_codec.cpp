#include "protocol/codec.hpp"
#include "domain/entry_schema.hpp"

#include <type_traits>


namespace anime::protocol {
using namespace anime::domain;
using namespace anime::schema;
using namespace anime::storage;
using anime::filtering::filterProcessor;

EntryRequest EntryWSC::decode(ACTION_TYPE action, const ByteVector& payload) {
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
				updateReq.updates.push_back(readUpdate(reader));
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
ByteVector EntryWSC::encode(const EntryResponse& response) {
	return std::visit([&](const auto& resp) { return encodeResponse(resp); }, response.payload);
}

Entry EntryWSC::readEntry(ByteReader& reader, std::vector<size_t>& fieldIndices) {
	Entry result;

	for (size_t i = 0; i < fieldIndices.size(); i++) {
		std::visit([&](const auto& memberPtr) {
			using FieldType = std::remove_cvref_t<decltype(result.*memberPtr)>;
			result.*memberPtr = reader.read<FieldType>();
		}, entryMetadata[fieldIndices[i]].ptr);
	}
	return result;
}

void EntryWSC::encodeEntry(ByteWriter& encoder, const Entry& entry) {
	for (size_t i = 0; i < entryMetadata.size(); i++) {
		std::visit([&](const auto& memberPtr) {
			encoder.write(entry.*memberPtr);
		}, entryMetadata[i].ptr);
	}
}

ByteVector EntryWSC::encodeResponse(const EntryGetResponse& response) {
	ByteWriter encoder;

	encodeSchema(encoder, response.schema);
	encoder.write(helpers::checkedConvert<std::uint32_t>(response.entries.size()));
	for (const Entry& entry : response.entries) {
		encodeEntry(encoder, entry);
	}

	return encoder.buffer;
}
ByteVector EntryWSC::encodeResponse(const EntryIdsResponse& response) {
	ByteWriter encoder;

	encoder.write(response.ids);

	return encoder.buffer;
}
ByteVector EntryWSC::encodeResponse(const EntrySuccessResponse& response) {
	ByteWriter encoder;

	encoder.write(response.message);

	return encoder.buffer;
}
ByteVector EntryWSC::encodeResponse(const EntryErrorResponse& response) {
	ByteWriter encoder;

	encoder.write(response.message);

	return encoder.buffer;
}



} // namespace anime::protocol
