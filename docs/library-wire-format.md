# Library WebSocket payloads

Use the existing application header with the library domain and a `LIBRARY_ACTION` byte.
The response echoes the request ID and action. Integers are big-endian. Strings have a U32 byte
length followed by their bytes; lists have a U32 element count. IDs are U32.

| Action | Request body | Success body |
| --- | --- | --- |
| GET_ANIME (1) | Group IDs | Group list |
| GET_ENTRIES (2) | Entry IDs | Entry list |
| CREATE_ANIME (3) | Name, description, input entry list | Group ID |
| ADD_ENTRIES (4) | Group ID, input entry list | Entry IDs |
| UPDATE_ANIME (5) | Group updates, entry updates | Empty |
| UPDATE_ENTRIES (6) | Entry updates | Empty |
| TRANSFER_ENTRY (7) | Entry ID, destination group ID | Empty |
| DELETE_ANIME (8) | Group IDs | Empty |
| DELETE_ENTRIES (9) | Entry IDs | Empty |

Every response body starts with U8 status: `1` introduces the success body above;
`0` introduces a U8 library error code and a string message.

Input entry fields, in order: title (string), alt_title (string), format (U8),
status (U8), genres (U8 list), channels (U8 list), audios (U8 list),
subtitles (U8 list), fan_service (U8), episode_count (U16), min_runtime (U32),
characters (U32 list). Output entries prepend ID and rank (U32) and append parent group ID.

Output group fields, in order: ID (U32), rank (U32), name and description (strings),
members and genres (U32 lists), knownRuntimeMinutes (U64), episodeCount, movieCount,
seriesCount (U32), fanServiceLevel and status (U8), runtimeComplete (U8 bool),
compiledFromEntryRevision (U64), compilationVersion (U32).

Each update contains a record ID, U8 operation, U8 field index, U8 value type, and
the value using that type. The existing update enums define the tags. A clear
operation still carries a typed value, which the service ignores. No schema
or field names are sent.
