#include "databases/entry_database.hpp"
#include "databases/group_database.hpp"

#include <cassert>
#include <type_traits>

int main() {
	static_assert(std::is_same_v<anime::domain::EntryId, std::uint32_t>);
	static_assert(std::is_same_v<anime::domain::GroupId, std::uint32_t>);
	assert(anime::domain::entryMetadata.size() == 15);
	assert(anime::domain::groupMetadata.size() == 15);
	assert(anime::domain::entryMetadata[0].info.uniqueness == anime::schema::UNIQUENESS::ALL_VALUES);
	assert(anime::domain::entryMetadata[0].info.editPolicy == anime::schema::EDIT_POLICY::INTERNAL_ONLY);
	for (const auto& field : anime::domain::groupMetadata) {
		if (field.rule)
			assert(field.info.editPolicy == anime::schema::EDIT_POLICY::INTERNAL_ONLY);
	}
	assert(anime::domain::entryFieldNames.at("title") == 2);
	return 0;
}
