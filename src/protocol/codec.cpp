#include "codec.hpp"
#include "entry_messages.hpp"

#include <type_traits>


namespace anime::protocol {

using namespace anime::schema;
using namespace anime::storage;

Update BaseWSC::readUpdate(ByteReader& reader) {
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
schemaData BaseWSC::readSchema(ByteReader& reader) {
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
sectionHeader BaseWSC::readSection(ByteReader& reader) {
	sectionHeader header;

	header.type = static_cast<sectionType>(reader.read<std::uint8_t>());
	header.title = reader.read<std::string>();
	header.recordCount = reader.read<std::uint32_t>();
	header.fieldCount = reader.read<std::uint32_t>();
	header.hasSchema = static_cast<bool>(reader.read<std::uint8_t>());

	return header;
}
void BaseWSC::encodeSection(ByteWriter& encoder, const sectionHeader& section) {
	encoder.write(static_cast<uint8_t>(section.type));
	encoder.write(section.title);
	encoder.write(section.recordCount);
	encoder.write(section.fieldCount);
	encoder.write(static_cast<uint8_t>(section.hasSchema));
}
void BaseWSC::encodeSchema(ByteWriter& encoder, const schemaData& schema) {
	encoder.write(helpers::checkedConvert<std::uint32_t>(schema.size()));

	for (const auto& field : schema) {
		encoder.write(field.name);
		encoder.write(static_cast<uint8_t>(field.type));
		encoder.write(static_cast<uint8_t>(field.mode));
	}
}

}
