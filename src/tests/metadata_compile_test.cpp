#include "databases/entry_data.hpp"
#include "databases/groups.hpp"

#include <cassert>
#include <type_traits>

int main() {
	static_assert(std::is_same_v<anime::domain::EntryId, std::uint32_t>);
	static_assert(std::is_same_v<anime::domain::GroupId, std::uint32_t>);
	assert(anime::domain::entryMetadata.size() == 15);
	assert(anime::domain::groupMetadata.size() == 4);
	assert(anime::domain::entryFieldNames.at("title") == 2);
	return 0;
}
