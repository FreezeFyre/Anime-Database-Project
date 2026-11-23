# Existing data contract

Source inspected 2026-11-22. This summarizes current types and policies; it does not approve a new schema.

## Identity and relationships

`src/domain/types.hpp` defines EntryId, GroupId, LookupId, UserId, IdentityId and SessionId as uint32
aliases. They identify records, not vector positions. The aliases do not provide compile-time separation.

`src/domain/entry.hpp` defines Entry: id, rank, title/alternate title, format/status, lookup-backed
metadata vectors, fan-service value, episode count, runtime, characters and the `ip_group` link.
Inspect `entryMetadata` and `src/domain/entry_schema.cpp` for exact widths and validation rules.

`src/domain/group.hpp` defines Group with id, rank, name, description, member entry IDs and compiled
genres, runtime, counts, status, fan-service, completeness and compilation provenance. Membership must
remain consistent with Entry.ip_group. Startup resolves memberships and compiles groups in `src/main.cpp`;
mutations coordinate both stores through LibraryService. This is a contract to verify, not a blanket
claim that every failure path preserves it.

Storyline grouping and rank semantics are documented in [storyline-groups.md](storyline-groups.md).
Zero rank means unranked. Do not infer rank from row order or spread it across a group's entries.

`src/domain/user.hpp` separates User from UserIdentity. Roles are VIEWER, EDITOR and ADMIN; identity
includes provider, login key, optional external identity information and password hash. Provider enum
values do not establish complete OAuth support. Current Entry/Group structs have no owner field;
do not infer implemented per-user ownership from Alpha's acceptance requirements.

## Storage and derived values

Data is held in memory and serialized to custom schema-bearing text `.dat` files, not SQL tables.
Startup currently loads `animeDatabase.crunchyroll.dat`, `animeLookupTables.1.dat`,
`groupDatabase.crunchyroll.dat` and `userDatabase.1.dat` from `src/io`.
Relevant persistence methods are entryDB.saveEntries, groupDB.save and userDB.save.
`src/io/saveFile.tpp` writes a temporary file and replaces/moves the target; inspect platform behavior
and error propagation before making durability claims. There is no demonstrated multi-file transaction.

LibraryService mutation paths inspected for this integration do not invoke entry/group save methods.
User account creation invokes save in `src/databases/user_database.cpp`. Alpha still requires explicit
verification of successful save/restart and caller-visible save failures.

Group compilation rules live in `src/domain/group.hpp` and `src/domain/group_compiler.cpp`; do not
duplicate derived-value rules in a client. Rank fields exist; a configurable weighted scoring system
is not established by the kit's example prompts.

## Modeling cautions

Optional blank imported scalars can become struct defaults on load/save; unknown and zero are not
universally preserved as distinct states. See storyline-groups.md for import schema compatibility
cautions. Check metadata, schema and codec representations together before modifying fields.

Consult data_architect for proposed identity, membership, stored scoring, schema or query changes.
Require compatibility, conversion bounds, migration/backfill and failure behavior appropriate to the
actual `.dat` stores. Do not invent catalog contents or impose a relational database without approved scope.
