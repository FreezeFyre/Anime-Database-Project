#include "databases/library_service.hpp"
#include "databases/user_database.hpp"
#include "protocol/codec.hpp"
#include <cassert>
#include <chrono>
#include <fstream>

namespace {
	using namespace anime::domain;
	using namespace anime::schema;
	using namespace anime::storage;
	using namespace anime::protocol;

	template <typename F> void rejects(F action) {
		bool failed = false;
		try {
			action();
		} catch (const std::exception&) {
			failed = true;
		}
		assert(failed);
	}

	void mutations() {
		entryDB entries;
		Entry first;
		first.title = "First";
		first.rank = 1;
		auto a = entries.addEntry(first, USER_ACCOUNT_ROLE::ADMIN);
		Entry second;
		second.title = "Second";
		auto b = entries.addEntry(second, USER_ACCOUNT_ROLE::ADMIN);
		assert(entries.getSize() == 2 && entries.getEntries({a})[0].rank == 1);
		auto title = [](EntryId id, std::string value) {
			return EntryUpdate{id, UPDATE_OPERATION::SET_FIELD, 2, dataType::STR, std::move(value)};
		};
		assert(entries.getEntries({b})[0].title == "Second");
		entries.updateEntry({title(a, "First")}, USER_ACCOUNT_ROLE::ADMIN);
		entries.updateEntry({title(a, ""), title(b, "")}, USER_ACCOUNT_ROLE::ADMIN);
		entries.updateEntry({{b, UPDATE_OPERATION::SET_FIELD, 13, dataType::VECU32, std::vector<std::uint32_t>{1}},
		                     {b, UPDATE_OPERATION::ADD_VALUE, 13, dataType::U32, std::uint32_t{2}}},
		                    USER_ACCOUNT_ROLE::ADMIN);
		assert((entries.getEntries({b})[0].characters == std::vector<std::uint32_t>{1, 2}));
		for (auto op : {UPDATE_OPERATION::SET_FIELD, UPDATE_OPERATION::CLEAR_FIELD, UPDATE_OPERATION::ADD_VALUE,
		                UPDATE_OPERATION::REMOVE_VALUE})
			rejects(
			    [&] { entries.updateEntry({{a, op, 0, dataType::U32, std::uint32_t{99}}}, USER_ACCOUNT_ROLE::ADMIN); });
		// TODO: Add duplicate insert/update and validation-scope regressions once uniqueness is implemented.

		groupDB groups;
		Group group;
		group.id = 999;
		group.members = {a};
		group.episodeCount = 999;
		auto g = groups.addGroup(group);
		assert(g != 999 && groups.getGroups({g})[0].members.empty());
		assert(groups.getGroups({g})[0].episodeCount == 0);
		for (size_t i = 0; i < groupMetadata.size(); ++i) {
			if (groupMetadata[i].info.editPolicy != EDIT_POLICY::INTERNAL_ONLY)
				continue;
			for (auto op : {UPDATE_OPERATION::SET_FIELD, UPDATE_OPERATION::CLEAR_FIELD, UPDATE_OPERATION::ADD_VALUE,
			                UPDATE_OPERATION::REMOVE_VALUE})
				rejects([&] {
					groups.updateGroups({{g, op, static_cast<std::uint8_t>(i), dataType::U32, std::uint32_t{1}}});
				});
		}
		std::shared_mutex em, gm;
		LibraryService library(entries, groups, em, gm);
		anime::net::Session session;
		session.role = USER_ACCOUNT_ROLE::ADMIN;
		Entry child;
		child.title = "Child";
		const auto added = library.addEntries(AddEntriesRequest{g, {child}}, session);
		assert(added && added->size() == 1);
		// TODO: Test cross-group and same-group transfers after the membership path is updated.
	}

	void parsing(const std::filesystem::path& dir) {
		fileProcessor parser;
		// TODO: Reject duplicate IDs, nonzero ranks and nonempty titles when loading, including reordered schemas.

		entryDB entries;
		Entry entry;
		entry.title = "Saved";
		auto id = entries.addEntry(entry, USER_ACCOUNT_ROLE::ADMIN);
		entries.saveEntries(dir / "saved.dat");
		entryDB reloaded;
		reloaded.loadEntries(dir / "saved.dat");
		assert(reloaded.getEntries({id})[0].title == "Saved");
		Entry invalid;
		invalid.fan_service = 11;
		EntryWSC codec;
		EntryGetResponse payload;
		payload.schema = entries.getSchema();
		payload.entries = {invalid};
		const auto bytes = codec.encode(EntryResponse{payload, false});
		rejects([&] { codec.decode(ACTION_TYPE::ACT_CREATE, bytes); });
		{
			std::ofstream file(dir / "bounds.dat");
			file << "ADP0|1002|0|1|\n0|ANIME_DATA|1|5|1|\n[id:3;0,rank:3;3,title:5;0,ip_group:3;2,fan_service:1;1]\n{1,"
			        "0,\"Bad\",0,11}\n";
		}
		rejects([&] {
			entryDB invalid;
			invalid.loadEntries(dir / "bounds.dat");
		});
		// TODO: Add group-load duplicate cases once collection uniqueness validation is implemented.
	}

	void accounts(const std::filesystem::path& dir) {
		userDB users;
		users.addUser({1, "One"});
		UserIdentity identity;
		identity.id = 1;
		identity.userId = 1;
		identity.loginKey = "one";
		identity.email = "one@example.com";
		users.addIdentity(identity);
		identity.id = 2;
		identity.loginKey = "one@example.com";
		identity.email.clear();
		rejects([&] { users.addIdentity(identity); });
		assert(users.getIdentityId_LoginKey("one@example.com") == 1);
		identity.loginKey = "two";
		users.addIdentity(identity);
		identity.id = 3;
		identity.loginKey = "three";
		users.addIdentity(identity);
		users.save(dir / "users.dat");
		userDB reloaded;
		reloaded.load(dir / "users.dat");
		assert(reloaded.getIdentityId_LoginKey("three") == 3);
		rejects([&] { reloaded.addIdentity(identity); });
		AuthCreateAccountRequest account{LoginProvider::PASSWORD, "one@example.com", "", "", "Duplicate"};
		rejects([&] { reloaded.createAccount(account, "hash"); });
		assert(reloaded.getSection(fileType::HEADER_DATABASE, 0).recordCount == 1);
		account.loginKey = "four";
		reloaded.createAccount(account, "hash");
		assert(reloaded.getIdentityId_LoginKey("four") == 4);
		{
			std::ofstream file(dir / "identities.dat");
			file << "ADP0|1002|0|1|\n0|IDENTITY_DATA|2|5|1|\n"
			        "[id:3;0,user_id:3;0,provider:1;0,login_key:5;0,disabled:0;0]\n"
			        "{1,1,0,\"same\",0}\n{2,1,0,\"same\",0}\n";
		}
		rejects([&] {
			userDB invalid;
			invalid.load(dir / "identities.dat");
		});
	}
} // namespace

void fieldValidationTests() {
	const auto dir =
	    std::filesystem::temp_directory_path() /
	    ("field-validation-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
	std::filesystem::create_directory(dir);
	mutations();
	parsing(dir);
	accounts(dir);
	std::filesystem::remove_all(dir);
}
