# Repository Guidelines

## Project Structure & Module Organization

- `src/domain/`: entry, group, and user models, metadata, schema validation, and group compilation.
- `src/databases/`: in-memory collections, field helpers, validation, authentication, and the evolving library service.
- `src/app/`: application context and request handlers; `src/protocol/`: typed messages and entry codecs.
- `src/net/`: routing, binary framing, HTTP, WebSocket, and socket transport.
- `src/io/`: text persistence and `.dat` fixtures; `src/tests/`: standalone test executables.
- `docs/`: current request flow and proposed architecture. Distinguish implemented behavior from plans; parts of README.md describe older designs.

## Build, Test, and Development Commands

Use an MSVC x64 developer shell with CMake 3.20+, Ninja, and vcpkg. The manifest supplies Argon2 and CURL.

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows
cmake --build build
ctest --test-dir build --output-on-failure
ctest --test-dir build -R "entry_codec_test|library_service_test" --output-on-failure
```

These configure, build, run all registered tests, and run selected tests. Visual Studio's `x64-Debug` configuration is defined in `CMakeSettings.json`.

Run `.\build\anime_database_project.exe` from the repository root so `src/io` resolves correctly. Startup currently launches networking unconditionally; use test executables for isolated checks.

## Coding Style & Naming Conventions

Use C++23. Follow `.editorconfig` and `.clang-format`: UTF-8, LF, final newline, tabs of width four, attached braces, and a 120-column limit. Format changed C++ files with `clang-format -i <files>`.

Follow surrounding camelCase methods and snake_case module filenames, such as `entry_database.hpp`. Preserve existing public names. Use explicit fixed-width types for stored/protocol values and `size_t` for container positions. Keep IDs distinct from positions and range-check narrowing conversions.

Classes, Structs, and other data types should be PascalCase such as `LibraryService`. Type specified enums should be all caps seperated with _ like `VALIDATION_SCOPE`.

## Testing Guidelines

Tests use standalone executables, CTest, and standard assertions; prefer Debug builds so assertions remain enabled. Name tests `*_test.cpp` and register targets in CMakeLists.txt. Add focused behavioral regression cases for changed logic; no coverage percentage is configured.

Use temporary directories for save/reload tests. Preserve checked-in and personal database files, existing edits, and generated build files.

## Commit & Pull Request Guidelines

History mixes descriptive messages with `refactor:` prefixes. Prefer concise, scoped messages such as `fix: preserve group membership after entry transfer`.

PRs should explain resulting behavior, relevant architecture/schema changes, and validation performed. Link related issues when applicable and identify existing failures separately.

## Target Development Goals

Alpha is the active milestone. Later milestones describe intended improvements, not prerequisites for Alpha. Add behavioral regression tests alongside each milestone.

### Alpha: Working Client and Library Workflow

- **Goal:** A client connects, authenticates, and creates, retrieves, updates, and deletes anime groups and their child entries.
- **Acceptance:** Returned IDs identify stored records; ownership, memberships, and compiled group values remain consistent after mutations. Unauthorized requests fail, and invalid requests leave existing data consistent.
- **Persistence:** Changes survive a successful save and graceful server restart. Save failures reach the caller. Document when changes are persisted.
- **Verification:** Automate permissions, creation, retrieval, updates, deletion, membership, compilation, and save/reload checks using temporary files. Demonstrate the same workflow through a real client.
- **Deferred:** Crash recovery, OAuth completion, generalized database frameworks, and performance redesigns.

### Beta: Shared Database Logic

- **Goal:** Reduce duplicated database logic by extracting proven common behavior, using templates where useful.
- **Acceptance:** Entry and group databases reuse suitable storage, conversion, or metadata helpers while regression tests preserve behavior. Keep entity-specific rules explicit.
- **Deferred:** Generalizing speculative requirements or maximizing template usage.

### Delta: Consistent Errors and Diagnostics

- **Goal:** Standardize meaningful error codes, logging context, and operation results. Use `std::expected` and `std::unexpected` where they clarify expected failures.
- **Acceptance:** Callers can distinguish permission, validation, missing-record, and persistence failures; diagnostics identify the failed operation and relevant record/request.
- **Scope:** Basic failure reporting is required in Alpha; this milestone improves consistency and diagnostic detail.

### Epsilon: Bounded Network Concurrency

- **Goal:** Support many clients with bounded thread and memory use.
- **Acceptance:** Establish a repeatable workload and capacity target, then measure correctness, resource use, responsiveness, and clean shutdown.
- **Approach:** Use non-blocking sockets with I/O readiness handling; introduce a bounded worker pool for queued work. Define ownership and synchronization before sharing tasks.

### Zeta: Measured Performance Improvements

- **Goal:** Profile representative workloads and optimize demonstrated bottlenecks, considering SIMD where appropriate.
- **Acceptance:** Compare against a documented baseline; optimized and scalar implementations produce equivalent results, with measured performance gains.
- **Deferred:** SIMD changes without profiling evidence or a representative benchmark.
