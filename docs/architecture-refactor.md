# Architecture refactor guide

## What changed in steps 1–3

The root namespace is `anime`. Domain records and metadata live under `domain`; repositories and file formats under `storage`; request/response values and codecs under `protocol`; authentication behavior under `auth`; routing and shared application ownership under `app`; and transport under `net`.

Namespaces prevent unrelated names from colliding. They are not a substitute for encapsulation: putting a large database class in a namespace does not make its responsibilities smaller. This refactor therefore also separates headers and translation units.

`core.hpp` has been removed. Include the header that owns the information you need:

| Need | Header |
| --- | --- |
| IDs and account roles | `domain/types.hpp` |
| Entry, user, or group models | `domain/entry.hpp`, `domain/user.hpp`, `domain/group.hpp` |
| Field presence, references, bounds, and update operations | `schema/field.hpp` |
| File-format value types, without a parser | `io/file_types.hpp` |
| Message version and action/domain enums | `protocol/message_types.hpp` |
| HTTP methods and response values | `net/http/http_types.hpp` |

Entry code now has separate ownership:

| Component | Responsibility |
| --- | --- |
| `storage::entryDB` | Collection, indexes, validation, queries, current persistence hooks |
| `domain::validateEntrySchema` | Stateless schema-to-entry-field mapping |
| `protocol::EntryWebSocketCodec` | Bytes to entry requests; entry responses to bytes |
| `app::EntryRouter` | Request dispatch, existing locking, repository calls, error conversion |
| `app::Context` | Existing database instances and mutexes |

The entry codec no longer needs `Context`, a session, or a request ID. The envelope codec owns request IDs, while authorization remains in the request-handling path. For example:

```cpp
anime::protocol::EntryWebSocketCodec codec;
auto request = codec.decode(anime::protocol::ACTION_TYPE::ACT_GET, payload);
```

The current wire layouts, numeric enum values, permission checks, and error messages are preserved. Entry operations are still a WebSocket API; HTTP entry requests remain unsupported. Plain records and variant-based request dispatch remain appropriate—there is no need to turn every DTO into a polymorphic class.

## Step 4: separate persistence from repositories

Today each repository also implements the file parser's callback interface: `section`, `schema`, `record`, and `table`. Its public API therefore exposes how the data is stored, not just what callers can do with the data. It also stores temporary parsing state such as field maps.

A future `EntryFileStore` should translate a file into an entry snapshot; `entryDB` should own and index that snapshot. A `UserFileStore` would do the corresponding work for users and identities. The parser can retain its existing concepts: a dedicated adapter can implement its callbacks instead of forcing the repository to implement them.

The important separation is:

```text
file parser → persistence adapter → validated snapshot → repository
```

Loading should parse and validate a temporary snapshot before replacing the live collection. Otherwise a malformed file encountered halfway through loading can leave a partially replaced database. Saving should serialize a consistent snapshot; writing to a temporary sibling file and replacing the destination only after success can prevent an interrupted save from destroying the previous version.

This makes file parsing, repository behavior, and future storage implementations independently testable. It does not require a base-class hierarchy. Start with concrete composed objects; add an interface only when there is another backend or a useful test seam.

## Step 5: put synchronization and authorization at explicit boundaries

Currently `Context` owns both a repository and its mutex. Callers must remember to acquire the matching mutex. A new route can accidentally call the repository without locking, and references returned by user queries can outlive the lock protecting them.

A repository can instead own its mutex privately and expose operations that acquire it. Read methods should return copies or immutable snapshots, not references into a vector that another thread can reallocate. Internal unlocked helpers can be private so a public operation can validate, update records, and repair indexes under one lock without recursively locking the same mutex.

Do not lock each tiny mutation independently when the complete operation must be atomic. Creating an account involves a user, an identity, and indexes: that whole use case needs one consistent transaction boundary. Operations spanning multiple repositories need an application-level coordinator and a defined locking order, or a broader transaction mechanism.

Authorization is a separate concern. An `EntryService` can accept an authenticated actor, check permissions, then call the repository. This avoids spreading permission logic across HTTP handlers, WebSocket handlers, and background jobs. A public repository API that accepts an arbitrary role is not itself proof that the caller actually has that role.

These changes are deliberately deferred. The current mutex scope and role checks remain intact in this refactor, avoiding a concurrency/authorization redesign during a file-organization change.

## Step 6: use RAII for sockets, runtimes, and worker lifetime

RAII means resource ownership is tied to an object's lifetime. A move-only socket wrapper closes its handle in its destructor. Returning early after a failed handshake then releases the connection automatically, rather than relying on every branch to call `closeSocket` correctly.

Use distinct owners for process-level socket/CURL initialization, listening sockets, and accepted client sockets. Keep copying disabled for exclusive handles; moving transfers ownership. Error-handling code should not clean up a process-wide runtime while other connections still depend on it.

Detached workers are the larger lifetime problem: they capture `this` but the service cannot wait for them before it is destroyed. Track workers with `std::jthread` or a bounded worker pool, request cancellation, and join them before releasing shared state. A stop token alone does not interrupt blocking `accept` or `recv`: shutdown must also wake those operations, for example through an explicit socket shutdown/close strategy or bounded polling.

Separate `HttpConnectionHandler` and `WebSocketConnectionHandler` from listener ownership. Handlers can own protocol-specific buffering and exchange rules; the listener accepts connections and controls lifetime. Tests should cover bind failure, handshake failure, client disconnect, shutdown while blocked, repeated start/stop, and destruction with active workers.

## Step 7: express architecture in CMake targets

The current build still lists implementation sources in executable targets. The next step is to create library targets representing actual dependency boundaries, then link the server, debug tools, and tests to those targets.

For example, `anime_domain` can own stateless domain validation; `anime_storage` can depend on domain; `anime_protocol` can own codecs and depend on domain/filtering; `anime_application` can depend on storage and request types; and `anime_network` can invoke application routing. Do not force a simple layer diagram if it creates a cycle—split shared message types from application behavior when necessary.

Use `PUBLIC` dependencies only when a library's public headers require them, and `PRIVATE` for implementation-only libraries. Attach include paths, language requirements, platform libraries, and compiler options to the owning targets instead of global configuration where practical.

This prevents tests from accidentally compiling an incomplete subset of production code and reduces repeated compilation. The standalone `entry_codec_test` added here is already a useful boundary check: it links no repository, authentication, or socket implementation. A future library-target conversion can preserve that same property without repeating source lists.

## Verification and remaining boundaries

The tests cover parser stress/fuzz cases, password hashing and user identity storage, simultaneous entry/group metadata includes, all entry request and response variants, stateless schema validation, existing fixture round trips, routing permission outcomes, WebSocket envelope fields, and HTTP entry rejection.

Malformed/truncated binary input, network shutdown, OAuth completion, and persistent authenticated HTTP sessions are not redesigned here. The existing `getTargetQueries` helper also remains unfinished; its compiler warning is pre-existing and should be addressed in a separate behavior-focused change. The unfinished HTTP client is still outside the executable source lists, as before.
