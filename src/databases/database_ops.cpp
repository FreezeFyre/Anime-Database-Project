#include "database_ops.hpp"

namespace anime::storage::helpers {

void validateField(const FieldInfo& fieldInfo, bool value) {
	if (fieldInfo.type != schemaField::dataType::BOOL) {
		throw std::runtime_error(" Field does not expect a boolean value. ");
	}
}
void validateField(const FieldInfo& fieldInfo, const std::string& value) {
	if (fieldInfo.type != schemaField::dataType::STR) {
		throw std::runtime_error(" Field does not expect a string value. ");
	}
}
void validateField(const FieldInfo& fieldInfo, const std::monostate value) {
	return;
}
void validateField(const FieldInfo& fieldInfo, const recordField& record) {
	std::visit([&](const auto& value) { validateField(fieldInfo, value); }, record);
}

} // namespace anime::storage::helpers
