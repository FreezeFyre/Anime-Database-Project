#include "users.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <utility>


//====================================
//				I/O
//====================================
void userDB::load(const std::string& filename) {
	fileProcessor f;
	try {
		f.readFile(filename, *this, fileType::HEADER_DATABASE);
		constructUserIdMap(userIdMap);
		constructIdentityIdMap(identityIdMap);
		passwordMap = constructKeyMap(LoginProvider::PASSWORD);
	} catch (const std::exception& e) {
		std::cerr << "Error Loading User Database: " << e.what() << "\n";
	}
}
void userDB::save(const std::string& filename) {
	fileProcessor f;
	try {
		f.saveFile(filename, *this, fileType::HEADER_DATABASE);
	} catch (const std::exception& e) {
		std::cerr << "Error Saving User Database: " << e.what() << "\n";
	}
}



//====================================
//			File Reading
//====================================
void userDB::section(const sectionHeader& section) {
	if (section.type != sectionType::SECTION_DATABASE_RECORDS) {
		throw std::runtime_error("Invalid section type for user database: " + std::to_string(static_cast<int>(section.type)));
		return;
	}
	if (section.title == "USER_DATA") {
		users.reserve(section.recordCount);
		return;
	} else if (section.title == "IDENTITY_DATA") {
		identities.reserve(section.recordCount);
		return;
	} else {
		throw std::runtime_error("Unknown section title for user database: " + section.title);
	}
}
void userDB::schema(const sectionHeader& section, const schemaData& schema) {
	if (section.title == "USER_DATA") {
		userFieldMap = validateUserSchema(schema);
	} else if (section.title == "IDENTITY_DATA") {
		identityFieldMap = validateIdentitySchema(schema);
	} else {
		throw std::runtime_error("Unknown section title for user database: " + section.title);
	}
}
void userDB::record(const sectionHeader& section, const recordData& record, const schemaData& schema) {
	if (section.title == "USER_DATA") {
		User user;

		for (size_t i = 0; i < userFieldMap.size(); i++) {
			try {
				validatePresence(record[i], UserMeta[userFieldMap[i]].info);
			}
			catch (const std::exception& e) {
				throw std::runtime_error("Error validating user record field \"" + std::to_string(userFieldMap[i]) + "\": " + e.what());
			}


			if (auto ptr = std::get_if<UserId User::*>(&UserMeta[userFieldMap[i]].ptr)) {
				if (auto val = std::get_if<int>(&record[i])) {
					user.*(*ptr) = static_cast<UserId>(*val);
				} else {
					throw std::runtime_error("Invalid data type in user record field " + std::to_string(i));
				}
			}
			else if (auto ptr = std::get_if<std::string User::*>(&UserMeta[userFieldMap[i]].ptr)) {
				if (auto val = std::get_if<std::string>(&record[i])) {
					user.*(*ptr) = *val;
				} else {
					throw std::runtime_error("Invalid data type in user record field " + std::to_string(i));
				}
			}
			else if (auto ptr = std::get_if<UserRole User::*>(&UserMeta[userFieldMap[i]].ptr)) {
				if (auto val = std::get_if<int>(&record[i])) {
					user.*(*ptr) = static_cast<UserRole>(*val);
				} else {
					throw std::runtime_error("Invalid data type in user record field " + std::to_string(i));
				}
			}
			else if (auto ptr = std::get_if<bool User::*>(&UserMeta[userFieldMap[i]].ptr)) {
				if (auto val = std::get_if<int>(&record[i])) {
					user.*(*ptr) = (*val != 0);
				} else {
					throw std::runtime_error("Invalid data type in user record field " + std::to_string(i));
				}
			} else {
				throw std::runtime_error("Unknown data type in user record field " + std::to_string(i));
			}

		}

		size_t index = users.size();
		users.emplace_back(std::move(user));
		userIdMap.emplace(static_cast<UserId>(users[index].id), index);
		nextUserId++;
	} else if (section.title == "IDENTITY_DATA") {
		UserIdentity identity;

		for (size_t i = 0; i < identityFieldMap.size(); i++) {
			try {
				validatePresence(record[i], UserIdentityMeta[identityFieldMap[i]].info);
			}
			catch (const std::exception& e) {
				throw std::runtime_error("Error validating identity record field \"" + std::to_string(identityFieldMap[i]) + "\": " + e.what());
			}

			if (auto ptr = std::get_if<uint32_t UserIdentity::*>(&UserIdentityMeta[identityFieldMap[i]].ptr)) {
				if (auto val = std::get_if<int>(&record[i])) {
					identity.*(*ptr) = *val;
				} else {
					throw std::runtime_error("Invalid data type in identity record field " + std::to_string(i));
				}
			}
			else if (auto ptr = std::get_if<LoginProvider UserIdentity::*>(&UserIdentityMeta[identityFieldMap[i]].ptr)) {
				if (auto val = std::get_if<int>(&record[i])) {
					identity.*(*ptr) = static_cast<LoginProvider>(*val);
				} else {
					throw std::runtime_error("Invalid data type in identity record field " + std::to_string(i));
				}
			}
			else if (auto ptr = std::get_if<std::string UserIdentity::*>(&UserIdentityMeta[identityFieldMap[i]].ptr)) {
				if (auto val = std::get_if<std::string>(&record[i])) {
					identity.*(*ptr) = *val;
				} else {
					throw std::runtime_error("Invalid data type in identity record field " + std::to_string(i));
				}
			}
			else if (auto ptr = std::get_if<bool UserIdentity::*>(&UserIdentityMeta[identityFieldMap[i]].ptr)) {
				if (auto val = std::get_if<int>(&record[i])) {
					identity.*(*ptr) = (*val != 0);
				} else {
					throw std::runtime_error("Invalid data type in identity record field " + std::to_string(i));
				}
			} else {
				throw std::runtime_error("Unknown data type in identity record field " + std::to_string(i));
			}
		}

		size_t index = identities.size();
		identities.emplace_back(std::move(identity));
		identityIdMap.emplace(static_cast<IdentityId>(identities[index].id), index);
		nextIdentityId++;
	} else {
		throw std::runtime_error("Unknown section title for user database: " + section.title);
	}
}
void userDB::table(const sectionHeader& section, const std::string& row) {
	throw std::runtime_error("User database does not contain lookup table records.");
}



//====================================
//			File Writing
//====================================
fileHeader userDB::getHeader(fileType type) const {
	if (type != fileType::HEADER_DATABASE) {
		throw std::runtime_error("Invalid file type for user database header.");
	}

	fileHeader result;
	result.type = type;
	result.sectionCount = 2;
	return result;
}
sectionHeader userDB::getSection(fileType type, size_t index) const {
	if (type != fileType::HEADER_DATABASE) {
		throw std::runtime_error("Invalid file type for user database section.");
	}
	if (index >= 2) {
		throw std::runtime_error("Invalid section index for user database.");
	}

	sectionHeader result;
	result.type = sectionType::SECTION_DATABASE_RECORDS;
	result.hasSchema = true;

	if (index == 0) {
		result.title = std::string("USER_DATA");
		result.recordCount = static_cast<uint32_t>(users.size());
		result.fieldCount = static_cast<uint32_t>(UserMeta.size());
	}
	else {
		result.title = std::string("IDENTITY_DATA");
		result.recordCount = static_cast<uint32_t>(identities.size());
		result.fieldCount = static_cast<uint32_t>(UserIdentityMeta.size());
	}

	return result;
}
schemaData userDB::getSchema(fileType type, size_t sectionIndex) const {
	schemaData result;
	schemaField buffer;

	if (sectionIndex == 0) {
		for (size_t i = 0; i < UserMeta.size(); i++) {
			buffer.name = UserMeta[i].info.name;
			buffer.type = UserMeta[i].info.type;
			buffer.mode = UserMeta[i].info.mode;

			result.push_back(buffer);
		}
	} else if (sectionIndex == 1) {
		for (size_t i = 0; i < UserIdentityMeta.size(); i++) {
			buffer.name = UserIdentityMeta[i].info.name;
			buffer.type = UserIdentityMeta[i].info.type;
			buffer.mode = UserIdentityMeta[i].info.mode;

			result.push_back(buffer);
		}
	} else {
		throw std::runtime_error("Invalid section index for user database schema.");
	}

	return result;
}
recordData userDB::getRecord(fileType type, size_t sectionIndex, size_t recordIndex) const {
	if (type != fileType::HEADER_DATABASE) {
		throw std::runtime_error("Invalid file type for user database record.");
	}

	recordData result;

	if (sectionIndex == 0) {
		for (size_t i = 0; i < UserMeta.size(); ++i) {
			recordField buffer;

			buffer = getUserField(users[recordIndex].id, i);
			result.push_back(buffer);
		}
	}
	else if (sectionIndex == 1) {
		for (size_t i = 0; i < UserIdentityMeta.size(); ++i) {
			recordField buffer;

			buffer = getIdentityField(identities[recordIndex].id, i);
			result.push_back(buffer);
		}
	}
	else {
		throw std::runtime_error("Invalid section index for user database.");
	}

	return result;
}
recordData userDB::getTableRecord(fileType type, size_t sectionIndex, size_t recordIndex) const {
	throw std::runtime_error("User database does not contain lookup table records.");
}



//====================================
//			Mutate Operations
//====================================
void userDB::addUser(const User& user) {
	if (userIdMap.find(user.id) != userIdMap.end()) {
		throw std::runtime_error("User with ID " + std::to_string(user.id) + " already exists.");
	}
	size_t index = users.size();
	users.push_back(user);
	userIdMap[user.id] = index;
}
void userDB::elevateUser(UserId id, UserRole newRole, const UserRole& role) {
	if (role != UserRole::ADMIN) {
		return;
	}
	auto it = userIdMap.find(id);
	if (it == userIdMap.end()) {
		throw std::runtime_error("User with ID " + std::to_string(id) + " does not exist.");
	}
	users[it->second].role = newRole;
}
void userDB::addIdentity(const UserIdentity& identity) {
	if (identityIdMap.find(identity.id) != identityIdMap.end()) {
		throw std::runtime_error("Identity with ID " + std::to_string(identity.id) + " already exists.");
	}
	size_t index = identities.size();
	identities.push_back(identity);
	identityIdMap[identity.id] = index;
}
void userDB::createAccount(const CreateAccountRequest& request, const std::string& argon_password) {
	User user;
	UserIdentity identity;
	

	// this all should work but i need to add duplicate verification and such, also need to add other login methods
	user.id = nextUserId;
	nextUserId++;
	identity.userId = user.id;
	identity.id = nextIdentityId;
	nextIdentityId++;

	user.displayName = request.displayName;
	identity.provider = request.provider;
	identity.loginKey = request.loginKey;

	switch (request.provider) {
		case LoginProvider::PASSWORD: {
			identity.passwordHash = argon_password;
			identity.email = request.email;
			passwordMap[request.email] = identity.id;
			passwordMap[request.loginKey] = identity.id;
			break;
		}
		default:
			throw std::runtime_error("Invalid login provider for creating a account \n");
	}

	addUser(user);
	addIdentity(identity);
}


//====================================
//			Query Operations
//====================================
const User& userDB::getUser(UserId id) const {
	auto it = userIdMap.find(id);
	if (it == userIdMap.end()) {
		throw std::runtime_error("User with ID " + std::to_string(id) + " does not exist.");
	}
	return users[it->second];
}
const User& userDB::getUser(size_t index) const {
	if (index >= users.size()) {
		throw std::runtime_error("User index " + std::to_string(index) + " is out of bounds.");
	}
	return users[index];
}
const UserIdentity& userDB::getIdentity(IdentityId id) const {
	auto it = identityIdMap.find(id);
	if (it == identityIdMap.end()) {
		throw std::runtime_error("Identity with ID " + std::to_string(id) + " does not exist.");
	}
	return identities[it->second];
}
const UserIdentity& userDB::getIdentity(size_t index) const {
	if (index >= identities.size()) {
		throw std::runtime_error("Identity index " + std::to_string(index) + " is out of bounds.");
	}
	return identities[index];
}
IdentityId userDB::getIdentityId_LoginKey(const std::string& loginKey) const {
	auto it = passwordMap.find(loginKey);
	if (it == passwordMap.end()) {
		throw std::runtime_error("User Identity key: " + loginKey + " does not exist. \n");
	}
	return it->second;
}




//====================================
//			Indexing
//=====================================
void userDB::constructUserIdMap(std::unordered_map<UserId, size_t>& userIdMap) const {
	userIdMap.clear();
	for (size_t i = 0; i < users.size(); i++) {
		userIdMap.emplace(users[i].id, i);
	}
}
void userDB::constructIdentityIdMap(std::unordered_map<IdentityId, size_t>& identityIdMap) const {
	identityIdMap.clear();
	for (size_t i = 0; i < identities.size(); i++) {
		identityIdMap.emplace(identities[i].id, i);
	}
}
std::unordered_map<std::string, IdentityId> userDB::constructKeyMap(LoginProvider provider) {
	std::unordered_map<std::string, IdentityId> result;

	for (const UserIdentity& identity : identities) {
		if (identity.provider == provider) {
			if (validateEmail(identity.email)) {
				result[identity.email] = identity.id;
			}
			result[identity.loginKey] = identity.id;
		} else {
			continue;
		}
	}
		
	return result;
}


//====================================
//			Validation
//====================================
bool userDB::validatePresence(const recordField& field, const FieldInfo& info) const {
	if (std::holds_alternative<std::monostate>(field) && info.presence == fieldPresence::REQUIRED) {
		throw std::runtime_error("Missing required data for field \"" + info.name + "\"");
	}
	return true;
}
std::vector<size_t> userDB::validateUserSchema(const schemaData& schema) const{
	std::vector<size_t> fieldIndices;
	std::unordered_set<std::string> seen;

	for (size_t i = 0; i < schema.size(); i++) {
		const schemaField& field = schema[i];

		if (!seen.insert(field.name).second) {
			std::cerr << "Duplicate schema field: \"" << field.name
				<< "\" at index: " << i << "\n";
			throw std::runtime_error("Duplicate schema field: " + field.name);
		}

		std::unordered_map<std::string, size_t>::const_iterator found = userNameMap.find(field.name);

		if (found == userNameMap.end()) {
			std::cerr << "Schema field: \"" << field.name
				<< "\" at index: " << i
				<< " not in UserMeta.\n";
			throw std::runtime_error("Schema field not found in UserMeta: " + field.name);
		}

		size_t metaIndex = found->second;

		if (UserMeta[metaIndex].info.type != field.type) {
			std::cerr << "Schema field: \"" << field.name
				<< "\" at index: " << i
				<< " has incorrect data type.\n";
			throw std::runtime_error("Schema field has incorrect data type: " + field.name);
		}

		fieldIndices.push_back(metaIndex);
	}

	for (size_t i = 0; i < UserMeta.size(); i++) {
		const FieldInfo& meta = UserMeta[i].info;

		if (!seen.contains(meta.name)) {
			if (meta.presence == fieldPresence::REQUIRED) {
				std::cerr << "Required field \"" << meta.name
					<< "\" is missing from schema.\n";
				throw std::runtime_error("Required field is missing from schema: " + meta.name);
			}
			else if (meta.presence == fieldPresence::RECOMMENDED) {
				std::clog << "Recommended field \"" << meta.name
					<< "\" is missing from schema.\n";
			}
			else {
				std::clog << "Optional field \"" << meta.name
					<< "\" is missing from schema.\n";
			}
		}
	}

	return fieldIndices;
}
std::vector<size_t> userDB::validateIdentitySchema(const schemaData& schema) const{
	std::vector<size_t> fieldIndices;
	std::unordered_set<std::string> seen;

	for (size_t i = 0; i < schema.size(); i++) {
		const schemaField& field = schema[i];

		if (!seen.insert(field.name).second) {
			std::cerr << "Duplicate schema field: \"" << field.name
				<< "\" at index: " << i << "\n";
			throw std::runtime_error("Duplicate schema field: " + field.name);
		}

		std::unordered_map<std::string, size_t>::const_iterator found = identityNameMap.find(field.name);

		if (found == identityNameMap.end()) {
			std::cerr << "Schema field: \"" << field.name
				<< "\" at index: " << i
				<< " not in UserIdentityMeta.\n";
			throw std::runtime_error("Schema field not found in UserIdentityMeta: " + field.name);
		}

		size_t metaIndex = found->second;

		if (UserIdentityMeta[metaIndex].info.type != field.type) {
			std::cerr << "Schema field: \"" << field.name
				<< "\" at index: " << i
				<< " has incorrect data type.\n";
			throw std::runtime_error("Schema field has incorrect data type: " + field.name);
		}

		fieldIndices.push_back(metaIndex);
	}

	for (size_t i = 0; i < UserIdentityMeta.size(); i++) {
		const FieldInfo& meta = UserIdentityMeta[i].info;

		if (!seen.contains(meta.name)) {
			if (meta.presence == fieldPresence::REQUIRED) {
				std::cerr << "Required field \"" << meta.name
					<< "\" is missing from schema.\n";
				throw std::runtime_error("Required field is missing from schema: " + meta.name);
			}
			else if (meta.presence == fieldPresence::RECOMMENDED) {
				std::clog << "Recommended field \"" << meta.name
					<< "\" is missing from schema.\n";
			}
			else {
				std::clog << "Optional field \"" << meta.name
					<< "\" is missing from schema.\n";
			}
		}
	}

	return fieldIndices;
}
bool userDB::validateEmail(const std::string& email) const {
	if (email.size() < 4 or email.empty()) {
		return false;
	}
	size_t pos = email.find('@');
	size_t dot_pos = email.find('.', pos);
	if (pos == std::string::npos) {
	// requires one @
		std::clog << email << " requires one @. \n";
		return false;
	}
	if (pos == email.size() - 1) {
		std::clog << email << " is invalid(to short) \n";
		return false;
	}

	if (dot_pos == std::string::npos) {
	// invalid email domain
		std::clog << email << " invalid domain(dosent contain .) \n";
		return false;
	}
    return true;
}


//====================================
//			Utilities
//====================================
recordField userDB::getUserField(size_t id, size_t fieldIndex) const {
	size_t index = userIdMap.at(id);
	return getFromUser(users[index], fieldIndex);
}
recordField userDB::getIdentityField(size_t id, size_t fieldIndex) const {
	size_t index = identityIdMap.at(id);
	return getFromIdentity(identities[index], fieldIndex);
}
// Replace code -> is made via ai 
recordField userDB::getFromUser(const User& user, const size_t fieldIndex) const {
	recordField result;

	const UserMemPtr& ptr = UserMeta[fieldIndex].ptr;

	std::visit([&](auto memberPtr) {
		using Member = std::remove_cvref_t<decltype(user.*memberPtr)>;

		if constexpr (std::is_same_v<Member, UserId>) {
			result = static_cast<int>(user.*memberPtr);
		}
		else if constexpr (std::is_same_v<Member, UserRole>) {
			result = static_cast<int>(user.*memberPtr);
		}
		else if constexpr (std::is_same_v<Member, bool>) {
			result = static_cast<int>(user.*memberPtr);
		}
		else {
			result = user.*memberPtr;
		}
	}, ptr);

	return result;
}
recordField userDB::getFromIdentity(const UserIdentity& identity, const size_t fieldIndex) const {
	recordField result;

	const UserIdentityMemPtr& ptr = UserIdentityMeta[fieldIndex].ptr;

	std::visit([&](auto memberPtr) {
		using Member = std::remove_cvref_t<decltype(identity.*memberPtr)>;

		if constexpr (std::is_same_v<Member, uint32_t>) {
			result = static_cast<int>(identity.*memberPtr);
		}
		else if constexpr (std::is_same_v<Member, LoginProvider>) {
			result = static_cast<int>(identity.*memberPtr);
		}
		else if constexpr (std::is_same_v<Member, bool>) {
			result = static_cast<int>(identity.*memberPtr);
		}
		else {
			result = identity.*memberPtr;
		}
	}, ptr);

	return result;
}
// End of ai code
std::unordered_map<std::string, size_t> buildIdentityFieldNames() {
	std::unordered_map<std::string, size_t> result;

	for (size_t i = 0; i < UserIdentityMeta.size(); i++) {
		result.emplace(UserIdentityMeta[i].info.name, i);
	}

	return result;
}
std::unordered_map<std::string, size_t> buildUserFieldNames() {
	std::unordered_map<std::string, size_t> result;

	for (size_t i = 0; i < UserMeta.size(); i++) {
		result.emplace(UserMeta[i].info.name, i);
	}

	return result;
}
