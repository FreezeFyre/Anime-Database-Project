#pragma once

#include "domain/group.hpp"

namespace anime::domain {
std::vector<size_t> validateGroupSchema(const anime::storage::schemaData& schema);
} // namespace anime::domain
