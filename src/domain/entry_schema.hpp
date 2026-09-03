#pragma once

#include "domain/entry.hpp"

namespace anime::domain {
std::vector<size_t> validateEntrySchema(const anime::storage::schemaData& schema);
} // namespace anime::domain
