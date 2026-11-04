#include "domain/entry_schema.hpp"
#include "protocol/codec.hpp"

#include <ranges>
#include <type_traits>

namespace anime::protocol {
	using namespace anime::domain;
	using namespace anime::schema;
	using namespace anime::storage;
	using anime::filtering::filterProcessor;

	LibraryRequest LibraryWSC::decode(LIBRARY_ACTION action, const ByteVector& payload) {
		ByteReader reader{payload};
		std::vector<size_t> groupFieldIndices{2, 3}; // Name and description.
		std::vector<size_t> entryFieldIndices{2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};

		const auto request = [&]() -> LibraryRequest {
			switch (action) {
				case LIBRARY_ACTION::GET_ANIME: {
					return GetAnimeRequest{.groups = reader.read<std::vector<GroupId>>()};
				}
				case LIBRARY_ACTION::GET_ENTRIES: {
					return GetEntriesRequest{.entries = reader.read<std::vector<EntryId>>()};
				}
				case LIBRARY_ACTION::CREATE_ANIME: {
					CreateAnimeRequest request;
					request.group = readGroup(reader, groupFieldIndices);

					const auto count = reader.read<std::uint32_t>();
					request.entries.reserve(count);

					for (std::uint32_t i = 0; i < count; ++i) {
						request.entries.push_back(readEntry(reader, entryFieldIndices));
					}

					return request;
				}
				case LIBRARY_ACTION::ADD_ENTRIES: {
					AddEntriesRequest request{.id = reader.read<GroupId>()};
					const auto count = reader.read<std::uint32_t>();
					request.entries.reserve(count);
					for (std::uint32_t i = 0; i < count; ++i) {
						request.entries.push_back(readEntry(reader, entryFieldIndices));
					}

					return request;
				}
				case LIBRARY_ACTION::UPDATE_ANIME: {
					UpdateAnimeRequest request;

					const auto groupCount = reader.read<std::uint32_t>();
					request.groupUpdates.reserve(groupCount);
					for (std::uint32_t i = 0; i < groupCount; ++i) {
						request.groupUpdates.push_back(readUpdate(reader));
					}
					const auto count = reader.read<std::uint32_t>();
					request.entryUpdates.reserve(count);
					for (std::uint32_t i = 0; i < count; ++i) {
						request.entryUpdates.push_back(readUpdate(reader));
					}

					return request;
				}
				case LIBRARY_ACTION::UPDATE_ENTRIES: {
					UpdateEntriesRequest request;
					const auto count = reader.read<std::uint32_t>();
					request.updates.reserve(count);
					for (std::uint32_t i = 0; i < count; ++i) {
						request.updates.push_back(readUpdate(reader));
					}

					return request;
				}
				case LIBRARY_ACTION::TRANSFER_ENTRY: {
					return TransferEntryRequest{.entry = reader.read<EntryId>(), .group = reader.read<GroupId>()};
				}
				case LIBRARY_ACTION::DELETE_ANIME: {
					return DeleteAnimeRequest{.groups = reader.read<std::vector<GroupId>>()};
				}
				case LIBRARY_ACTION::DELETE_ENTRIES: {
					return DeleteEntriesRequest{.entries = reader.read<std::vector<EntryId>>()};
				}
				default:
					std::cerr << "Unknown action type: " << static_cast<int>(action) << "\n";
					throw std::runtime_error("Unknown entry action type.");
			}
		}();
		if (reader.offset != payload.size()) {
			throw std::runtime_error("Trailing bytes in library request.");
		}
		return request;
	}
	ByteVector LibraryWSC::encode(const LibraryResponse& response) {
		return std::visit([&](const auto& resp) { return encodeResponse(resp); }, response);
	}

	Entry LibraryWSC::readEntry(ByteReader& reader, std::vector<size_t>& fieldIndices) {
		Entry result;

		for (size_t i = 0; i < fieldIndices.size(); i++) {
			std::visit(
				[&](const auto& memberPtr) {
					using FieldType = std::remove_cvref_t<decltype(result.*memberPtr)>;
					result.*memberPtr = reader.read<FieldType>();
				},
				entryMetadata[fieldIndices[i]].ptr);
		}
		return result;
	}
	Group LibraryWSC::readGroup(ByteReader& reader, std::vector<size_t>& fieldIndices) {
		Group result;

		for (size_t i = 0; i < fieldIndices.size(); i++) {
			std::visit(
				[&](const auto& memberPtr) {
					using FieldType = std::remove_cvref_t<decltype(result.*memberPtr)>;
					result.*memberPtr = reader.read<FieldType>();
				},
				groupMetadata[fieldIndices[i]].ptr);
		}
		return result;
	}

	void LibraryWSC::encodeEntry(ByteWriter& encoder, const Entry& entry) {
		for (const auto& field : entryMetadata) {
			std::visit([&](const auto& memberPtr) { encoder.write(entry.*memberPtr); }, field.ptr);
		}
	}
	void LibraryWSC::encodeGroup(ByteWriter& encoder, const Group& group) {
		for (const auto& field : groupMetadata) {
			std::visit([&](const auto& memberPtr) { encoder.write(group.*memberPtr); }, field.ptr);
		}
	}

	ByteVector LibraryWSC::encodeResponse(const Result<void>& response) {
		if (!response) {
			return encodeResponse(response.error());
		}
		ByteWriter encoder;

		encoder.write(true);

		return encoder.buffer;
	}
	ByteVector LibraryWSC::encodeResponse(const Result<GroupId>& response) {
		if (!response) {
			return encodeResponse(response.error());
		}
		ByteWriter encoder;

		encoder.write(true);
		encoder.write(*response);

		return encoder.buffer;
	}
	ByteVector LibraryWSC::encodeResponse(const Result<std::vector<GroupId>>& response) {
		if (!response) {
			return encodeResponse(response.error());
		}
		ByteWriter encoder;

		encoder.write(true);
		encoder.write(*response);

		return encoder.buffer;
	}
	ByteVector LibraryWSC::encodeResponse(const Result<std::vector<Entry>>& response) {
		if (!response) {
			return encodeResponse(response.error());
		}
		ByteWriter encoder;

		encoder.write(true);
		encoder.write(helpers::checkedConvert<std::uint32_t>(response->size()));
		for (const auto& entry : *response) {
			encodeEntry(encoder, entry);
		}

		return encoder.buffer;
	}
	ByteVector LibraryWSC::encodeResponse(const Result<std::vector<Group>>& response) {
		if (!response) {
			return encodeResponse(response.error());
		}
		ByteWriter encoder;

		encoder.write(true);
		encoder.write(helpers::checkedConvert<std::uint32_t>(response->size()));
		for (const auto& group : *response) {
			encodeGroup(encoder, group);
		}

		return encoder.buffer;
	}
	ByteVector LibraryWSC::encodeResponse(const ErrorResponse& response) {
		ByteWriter encoder;
		encoder.write(false);
		encoder.write(static_cast<std::uint8_t>(response.code));
		encoder.write(response.err);
		return encoder.buffer;
	}

} // namespace anime::protocol
