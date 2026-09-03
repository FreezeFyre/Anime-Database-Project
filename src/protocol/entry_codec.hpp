#pragma once

#include "protocol/entry_messages.hpp"
#include "protocol/message_types.hpp"
#include "net/protocol/data.hpp"

namespace anime::protocol {

class EntryWebSocketCodec {
	public:
		EntryRequest decode(ACTION_TYPE action, const ByteVector& payload);
		ByteVector encode(const EntryResponse& response);

	private:
		Entry readEntry(ByteReader& reader, std::vector<size_t>& fieldIndices);
		schemaData readSchema(ByteReader& reader);
		sectionHeader readSection(ByteReader& reader);
		EntryUpdate translateUpdate(const Update& update);

		void encodeSection(ByteWriter& encoder, const sectionHeader& section);
		void encodeSchema(ByteWriter& encoder, const schemaData& schema);
		void encodeEntry(ByteWriter& encoder, const Entry& entry);

		ByteVector encodeResponse(const EntryGetResponse& response);
		ByteVector encodeResponse(const EntryIdsResponse& response);
		ByteVector encodeResponse(const EntrySuccessResponse& response);
		ByteVector encodeResponse(const EntryErrorResponse& response);
};

} // namespace anime::protocol
