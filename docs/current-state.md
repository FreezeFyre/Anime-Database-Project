# Current implementation

Inspected 2026-11-22 at baseline commit `4083361`. This is a navigation aid from source inspection,
not a claim that Alpha acceptance criteria pass. Verify the relevant paths again before changing them.

## Stack and entry points

- C++23 backend; CMake 3.20+, vcpkg Argon2 and CURL dependencies: `CMakeLists.txt`, `vcpkg.json`.
- Application state holds in-memory entries, groups, users, sessions, filtering, and mutexes:
  `src/app/context.hpp`. This is not a SQL-backed catalog.
- `src/main.cpp` loads data from `src/io` relative to the working directory, resolves memberships,
  compiles groups, and starts NetService. The parsed `--server` flag does not gate networking.
- HTTP and WebSocket listeners use ports 8081 and 8080: `src/net/transport/net_service.hpp`.
- A local ignored `temp-portal` and checked-in Nginx configuration exist. Inspect them directly for
  client behavior; a completed library client workflow has not been demonstrated by this integration.

## Implemented code paths

- Typed entry and library requests flow through `src/net/routing/router.cpp`,
  `src/app/entry_handler.cpp`, and `src/databases/library_service.cpp`.
- LibraryService has group creation/update/deletion, child mutation/transfer, retrieval, search and
  filtering paths. Their presence does not prove authorization, atomicity, or persistence correctness.
- Domain metadata and schemas describe entries and groups; `src/domain/group_compiler.cpp` computes
  aggregate fields. `src/databases/validation.hpp` handles cross-record validation.
- Password authentication and session logic live in `src/databases/auth/`; users and identities are
  defined in `src/domain/user.hpp` and stored by `src/databases/user_database.cpp`.
- Crunchyroll import helpers are under `scripts/`; storyline grouping policy is in
  [storyline-groups.md](storyline-groups.md). Imports and accounts already have implementation.

## Persistence and verification limits

Entry/group save methods exist, but LibraryService mutations inspected here do not call them.
Account creation calls userDB save. Do not claim successful library mutations are durable or that
shutdown saves them without tracing and testing that behavior. See [data-contract.md](data-contract.md).

CMake registers standalone assertion-based tests for parser stress/fuzz, imports, auth, metadata,
entry/library codecs, entry integration, library service/WebSocket, and field validation. Use Debug
and the commands in AGENTS.md. Several registered checks share the tracked
`src/tests/entry_integration_test.cpp` source; keep that source available in clean checkouts.

The local portal, parser output directories, Nginx executable and Nginx runtime logs are excluded from
version control. Nginx configuration and the catalog/test fixtures remain tracked.

This workflow integration does not build or run the C++ application. It validates configuration and
documentation separately. Linux deployment instructions and systemd units exist under `deploy/`;
their presence is not evidence of a running deployment.

## Documentation boundaries

[request-response-map.md](request-response-map.md) is a detailed historical source map and can lag newer
library routing. [target-library-architecture.md](target-library-architecture.md) and
[architecture-refactor.md](architecture-refactor.md) include intended designs. Read source before
asserting any proposed component or older flow is current.
