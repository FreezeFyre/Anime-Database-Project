#include "../databases/entry_data.hpp"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr EntryId firstGeneratedId = 50;

Entry makeEntry() {
	Entry entry;
	entry.rank = 0;
	entry.title = "Original Title";
	entry.alt_title = "Original Alt";
	entry.format = 0;
	entry.status = 0;
	entry.genres = { 0 };
	entry.channels = { 0 };
	entry.audios = { 0 };
	entry.subtitles = { 0 };
	entry.fan_service = 1;
	entry.episode_count = 12;
	entry.min_runtime = 24;
	entry.characters = { 10 };
	entry.ip_group = 0;
	return entry;
}

entryDB makeDbWithEntry() {
	entryDB db;
	const std::filesystem::path lookupPath =
		std::filesystem::path(__FILE__).parent_path().parent_path() / "io" / "animeLookupTables.1.dat";

	db.loadLookupTables(lookupPath.string());
	db.addEntry(makeEntry(), UserRole::ADMIN);
	return db;
}

bool contains(const std::vector<int>& values, int needle) {
	return std::find(values.begin(), values.end(), needle) != values.end();
}

void expectThrows(const char* name, const std::function<void()>& action) {
	try {
		action();
		std::cerr << "Expected exception was not thrown: " << name << '\n';
		assert(false);
	}
	catch (const std::exception&) {
	}
}

void appliesSupportedUpdateOps() {
	entryDB db = makeDbWithEntry();

	db.updateEntry({
		EntryUpdate{ firstGeneratedId, UpdateOp::SET_FIELD, 2, std::string("Updated Title") },
		EntryUpdate{ firstGeneratedId, UpdateOp::SET_FIELD, 10, 5 },
		EntryUpdate{ firstGeneratedId, UpdateOp::ADD_VALUE, 6, std::vector<int>{ 1, 2 } },
		EntryUpdate{ firstGeneratedId, UpdateOp::REMOVE_VALUE, 6, std::vector<int>{ 0 } },
		EntryUpdate{ firstGeneratedId, UpdateOp::CLEAR_FIELD, 3, std::monostate{} },
	}, UserRole::ADMIN);

	const Entry updated = db.getEntries({ firstGeneratedId }).front();
	assert(updated.title == "Updated Title");
	assert(updated.fan_service == 5);
	assert(updated.alt_title.empty());
	assert(!contains(updated.genres, 0));
	assert(contains(updated.genres, 1));
	assert(contains(updated.genres, 2));
}

void rejectsUnsupportedOrInvalidUpdates() {
	entryDB db = makeDbWithEntry();

	expectThrows("id update", [&] {
		db.updateEntry({ EntryUpdate{ firstGeneratedId, UpdateOp::SET_FIELD, 0, 99 } }, UserRole::ADMIN);
	});

	expectThrows("rank update", [&] {
		db.updateEntry({ EntryUpdate{ firstGeneratedId, UpdateOp::SET_FIELD, 1, 1 } }, UserRole::ADMIN);
	});

	expectThrows("clear required field", [&] {
		db.updateEntry({ EntryUpdate{ firstGeneratedId, UpdateOp::CLEAR_FIELD, 2, std::monostate{} } }, UserRole::ADMIN);
	});

	expectThrows("add value to scalar field", [&] {
		db.updateEntry({ EntryUpdate{ firstGeneratedId, UpdateOp::ADD_VALUE, 10, std::vector<int>{ 1 } } }, UserRole::ADMIN);
	});

	expectThrows("lookup value out of range", [&] {
		db.updateEntry({ EntryUpdate{ firstGeneratedId, UpdateOp::SET_FIELD, 4, 99 } }, UserRole::ADMIN);
	});
}

void translatesNetworkUpdateValues() {
	entrySerializer serializer;

	Update scalar;
	scalar.id = firstGeneratedId;
	scalar.operation = UpdateOp::SET_FIELD;
	scalar.fieldIndex = 10;
	scalar.type = dataType::U8;
	scalar.value = static_cast<uint8_t>(7);

	const EntryUpdate scalarResult = serializer.translateUpdate(scalar);
	assert(scalarResult.id == firstGeneratedId);
	assert(scalarResult.op == UpdateOp::SET_FIELD);
	assert(scalarResult.fieldIndex == 10);
	assert(std::get<int>(scalarResult.value) == 7);

	Update vector;
	vector.id = firstGeneratedId;
	vector.operation = UpdateOp::ADD_VALUE;
	vector.fieldIndex = 6;
	vector.type = dataType::VECU8;
	vector.value = std::vector<int>{ 1, 2 };

	const EntryUpdate vectorResult = serializer.translateUpdate(vector);
	assert((std::get<std::vector<int>>(vectorResult.value) == std::vector<int>{ 1, 2 }));
}

}

int main() {
	appliesSupportedUpdateOps();
	rejectsUnsupportedOrInvalidUpdates();
	translatesNetworkUpdateValues();

	std::cout << "entry_update_test passed\n";
	return 0;
}
