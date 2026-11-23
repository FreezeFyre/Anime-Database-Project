# Architecture and repository boundaries

Source inspected 2026-11-22; describes existing responsibilities without approving a redesign.

| Location | Current responsibility |
| --- | --- |
| `src/domain/` | Entity types, metadata, entry/group schemas, group compilation |
| `src/databases/` | In-memory collections, validation, auth, LibraryService coordination |
| `src/app/` | Shared Context and application request handlers |
| `src/protocol/` | Typed requests/responses and binary codecs |
| `src/net/` | HTTP/WebSocket framing, transport, sessions, routing |
| `src/io/` | Text serialization, loading/saving, checked-in and personal data files |
| `src/filtering/`, `src/schema/` | Filtering helpers and field definitions |
| `src/tests/` | Standalone C++ test programs registered in CMake |
| `scripts/`, `deploy/` | Import/maintenance tools and deployment configuration |
| `.codex/`, `AGENTS.md` | Project agent configuration and repository instructions |
| `docs/` | Implementation context, policies, proposals and substantial decisions/plans |

## Request and state flow

NetService receives HTTP/WebSocket traffic; protocol codecs decode typed messages. Router dispatches
auth, entry and library requests. LibraryService references Context's entry/group stores and mutexes;
Context owns the stores. Domain schemas, validation and compilation support database operations.
See `src/net/routing/router.cpp`, `src/app/entry_handler.cpp`, `src/databases/library_service.hpp`,
and `src/app/context.hpp`. These are modules within one backend process.

File loading and saving go through storage interfaces in `src/io/`. A network mutation does not imply
a completed save. Locking both stores does not itself make multi-step mutations transactional.

## Organization guidance

Place files by responsibility using existing boundaries. For moves, update includes, CMake source
lists, tests, documentation links, scripts and relative runtime paths together. Preserve generated
build directories and personal data. Use project_maintainer for a bounded review of such changes.

## Existing design material

- [Request/response map](request-response-map.md): useful detailed flow; check for drift.
- [Target library architecture](target-library-architecture.md): intended boundaries, not proof of completion.
- [Architecture refactor](architecture-refactor.md): design material to compare with actual code.
- [Library wire format](library-wire-format.md): compare against codecs before changing the protocol.

No new runtime architecture decision is made by installing the agent workflow. Add consequential
approved decisions under `docs/decisions/` using the template; keep proposals explicitly proposed.
