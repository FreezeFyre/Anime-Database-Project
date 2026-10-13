#include "databases/entry_database.hpp"
#include "databases/group_database.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace anime::storage;
using namespace anime::domain;

static void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}

struct Capture {
    recordData fields;
    void section(const sectionHeader&) {}
    void schema(const sectionHeader&, const schemaData&) {}
    void record(const sectionHeader&, const recordData& r, const schemaData&) { fields = r; }
    void table(const sectionHeader&, const std::string&) {}
};

int main(int argc, char** argv) {
    try {
        require(argc == 4, "Usage: crunchyroll_import_test <entries> <groups> <scratch-directory>");
        const auto scratch = std::filesystem::path(argv[3]);
        std::filesystem::create_directories(scratch);
        const auto fixture = scratch / "blank-fields.dat";
        {
            std::ofstream file(fixture);
            file << "ADP0|1002|0|1|\n0|TEST|1|8|1|\n"
                 << "[text:5;0,number:3;0,zero:3;0,flag:0;0,list:8;0,missingList:8;0,last:2;0,trailing:1;0]\n"
                 << "{\"\",,0,,{},,17,}\n";
        }
        fileProcessor parser;
        Capture capture;
        parser.readFile(fixture, capture, fileType::HEADER_DATABASE);
        require(std::get<std::string>(capture.fields[0]).empty(), "Empty text must remain a string");
        for (auto i : {1, 3, 5, 7}) require(std::holds_alternative<std::monostate>(capture.fields[i]), "Blank non-text must be missing");
        require(std::get<std::uint32_t>(capture.fields[2]) == 0, "Explicit zero must not become missing");
        require(std::get<std::vector<std::uint32_t>>(capture.fields[4]).empty(), "Explicit empty vector must remain a vector");
        require(std::get<std::uint16_t>(capture.fields[6]) == 17, "Nonempty numeric regression");
        {
            std::ofstream file(fixture);
            file << "ADP0|1002|0|1|\n0|ANIME_DATA|1|3|1|\n[id:3;0,rank:3;3,title:5;0]\n{0,,\"Missing rank\"}\n";
        }
        bool rejected = false;
        try { entryDB bad; parser.readFile(fixture, bad, fileType::HEADER_DATABASE); }
        catch (const std::exception&) { rejected = true; }
        require(rejected, "Missing required numeric field must be rejected");

        entryDB entries;
        groupDB groups;
        parser.readFile(argv[1], entries, fileType::HEADER_DATABASE);
        parser.readFile(argv[2], groups, fileType::HEADER_DATABASE);
        require(entries.getSection().recordCount == 271, "Entry count");
        require(groups.getSection().recordCount == 180, "Storyline group count");
        std::vector<unsigned> memberships(271);
        unsigned ranked = 0, sao = 0, ggo = 0;
        for (unsigned id = 0; id < 180; ++id) {
            const auto name = std::get<std::string>(groups.getField(id, groupFieldNames.at("name")));
            const auto members = std::get<std::vector<std::uint32_t>>(groups.getField(id, groupFieldNames.at("members")));
            const auto totalRuntime = std::get<std::uint64_t>(groups.getField(id, groupFieldNames.at("knownRuntimeMinutes")));
            std::uint64_t runtime = 0;
            for (const auto member : members) {
                require(member < memberships.size(), "Dangling member");
                ++memberships[member];
                require(std::get<std::uint32_t>(entries.getField(member, entryFieldNames.at("ip_group"))) == id, "Membership mismatch");
                const auto title = std::get<std::string>(entries.getField(member, entryFieldNames.at("title")));
                const auto rank = std::get<std::uint32_t>(entries.getField(member, entryFieldNames.at("rank")));
                runtime += std::get<std::uint32_t>(entries.getField(member, entryFieldNames.at("min_runtime")));
                if (rank) { ++ranked; require(rank == 1 && title == "86 EIGHTY-SIX", "Incorrect rank transfer"); }
                if (name == "Sword Art Online") require(title.find("Alternative") == std::string::npos, "GGO mixed into SAO");
            }
            require(runtime == totalRuntime, "Runtime aggregate");
            require(std::get<std::uint32_t>(groups.getField(id, groupFieldNames.at("rank"))) == 0, "Invented group rank");
            if (name == "Sword Art Online") { ++sao; require(members.size() == 7, "SAO should include its two Progressive movies"); }
            if (name == "Sword Art Online Alternative: Gun Gale Online") { ++ggo; require(members.size() == 2, "Separate GGO seasons"); }
        }
        for (const auto count : memberships) require(count == 1, "Orphan or duplicate membership");
        require(ranked == 1 && sao == 1 && ggo == 1, "Ranks/storyline groups");
        std::cout << "PASS: blank-field regression, required-field rejection, 271 entries, 180 storyline groups, rank transfer and references.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
