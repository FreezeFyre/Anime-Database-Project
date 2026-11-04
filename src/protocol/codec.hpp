#pragma once

#include <domain/entry.hpp>
#include <domain/group.hpp>
#include <domain/types.hpp>
#include <vector>
#include "io/file_types.hpp"
#include "net/protocol/data.hpp"
#include "protocol/entry_messages.hpp"
#include "protocol/library_messages.hpp"
#include "protocol/message_types.hpp"


namespace anime::protocol {

using anime::storage::schemaData;
using anime::storage::sectionHeader;

class BaseWSC {
	protected:
		schemaData readSchema(ByteReader& reader);
		sectionHeader readSection(ByteReader& reader);
		Update readUpdate(ByteReader& reader);

		void encodeSection(ByteWriter& encoder, const sectionHeader& section);
		void encodeSchema(ByteWriter& encoder, const schemaData& schema);
};

class LibraryWSC : protected BaseWSC {
	Entry readEntry(ByteReader& reader, std::vector<size_t>& fieldIndices);
	Group readGroup(ByteReader& reader, std::vector<size_t>& fieldIndices);

	void encodeEntry(ByteWriter& encoder, const Entry& entry);
	void encodeGroup(ByteWriter& encoder, const Group& group);

	ByteVector encodeResponse(const Result<void>& response);
	ByteVector encodeResponse(const Result<GroupId>& response);
	ByteVector encodeResponse(const Result<std::vector<GroupId>>& response);
	ByteVector encodeResponse(const Result<std::vector<Entry>>& response);
	ByteVector encodeResponse(const Result<std::vector<Group>>& response);
	ByteVector encodeResponse(const ErrorResponse& response);

	public:
		LibraryRequest decode(LIBRARY_ACTION action, const ByteVector& payload);
		ByteVector encode(const LibraryResponse& response);
};

class EntryWSC : protected BaseWSC {
	Entry readEntry(ByteReader& reader, std::vector<size_t>& fieldIndices);

	void encodeEntry(ByteWriter& encoder, const Entry& entry);

	ByteVector encodeResponse(const EntryGetResponse& response);
	ByteVector encodeResponse(const EntryIdsResponse& response);
	ByteVector encodeResponse(const EntrySuccessResponse& response);
	ByteVector encodeResponse(const EntryErrorResponse& response);

	public:
		EntryRequest decode(ACTION_TYPE action, const ByteVector& payload);
		ByteVector encode(const EntryResponse& response);
};
};
