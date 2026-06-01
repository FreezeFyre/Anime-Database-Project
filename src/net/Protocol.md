# Anime Database Network Protocol

This document describes the custom binary protocol used by the Anime Database
server after a WebSocket connection has been established.

The protocol has two layers:

1. WebSocket transport, based on RFC 6455.
2. Anime Database messages, encoded as binary WebSocket frames.

The WebSocket layer gets bytes from the browser to the server. The Anime
Database message layer explains what those bytes mean.

## Transport

Clients connect to the server with a WebSocket URL such as:

```txt
ws://127.0.0.1:8080
```

The server performs a normal WebSocket HTTP upgrade handshake. After the
handshake succeeds:

- Client requests must be sent as binary WebSocket frames.
- Server responses are sent as binary WebSocket frames.
- WebSocket ping frames receive WebSocket pong frames.
- WebSocket close frames close the connection.
- Non-binary data frames are ignored by the current server.

## Byte Order

All multi-byte integer values are encoded in big-endian order.

For example, the decimal value `1001` is encoded as:

```txt
00 00 03 E9
```

This matches the current `ByteWriter::U16`, `ByteWriter::U32`,
`ByteReader::U16`, and `ByteReader::U32` implementations.

## Primitive Types

These are the basic wire types used by message payloads.

| Type | Size | Encoding |
| --- | ---: | --- |
| `U8` | 1 byte | Unsigned 8-bit integer |
| `U16` | 2 bytes | Unsigned 16-bit integer, big-endian |
| `U32` | 4 bytes | Unsigned 32-bit integer, big-endian |
| `CHAR` | 1 byte | Raw character byte |
| `STR` | variable | `U32 length`, then `length` raw character bytes |
| `VECU8` | variable | `U32 count`, then `count` `U8` values |
| `VECU16` | variable | `U32 count`, then `count` `U16` values |
| `VECU32` | variable | `U32 count`, then `count` `U32` values |

Strings are currently written as raw bytes from `std::string`. The protocol
does not currently declare a text encoding, but UTF-8 is the best target for
browser clients.

## Message Envelope

Every Anime Database message uses the same outer envelope:

```txt
ENCODE_VERSION  U32
COM_TYPE        U8
DOMAIN_TYPE     U8
ACTION_TYPE     U8
ID              U32
PAYLOAD_SIZE    U32
PAYLOAD         PAYLOAD_SIZE bytes
```

The envelope size before `PAYLOAD` is always 15 bytes.

### Fields

| Field | Meaning |
| --- | --- |
| `ENCODE_VERSION` | Protocol encoding version. Current value is `1001`. |
| `COM_TYPE` | High-level communication type: request, response, status, ping. |
| `DOMAIN_TYPE` | Which application domain should handle the message. |
| `ACTION_TYPE` | What operation should be performed in that domain. |
| `ID` | Request/response correlation ID chosen by the sender. |
| `PAYLOAD_SIZE` | Number of bytes in `PAYLOAD`. |
| `PAYLOAD` | Domain-specific binary payload. |

If `ENCODE_VERSION` does not match the server's expected version, decoding
throws an unsupported version error.

## Communication Types

| Name | Value | Meaning |
| --- | ---: | --- |
| `REQ` | `1` | Client request |
| `RESP` | `2` | Server response |
| `STATUS` | `3` | Status message, not fully defined yet |
| `PING` | `4` | Application-level ping message |

Note: WebSocket ping frames are handled separately from `COM_TYPE::PING`.

## Domains

| Name | Value | Meaning |
| --- | ---: | --- |
| `ENTRY` | `1` | Anime entry records and entry lookup data |
| `GROUPS` | `2` | Groups/franchises, planned |
| `CHARACTERS` | `3` | Character records, planned |
| `FILTERS` | `4` | Filter operations, planned |
| `AUTH` | `10` | Authentication, planned |

## Actions

| Name | Value | Meaning |
| --- | ---: | --- |
| `ACT_GET` | `1` | Read data |
| `ACT_UPDATE` | `2` | Update existing data, not fully implemented |
| `ACT_CREATE` | `3` | Create records |
| `ACT_DELETE` | `4` | Delete records, not implemented |
| `ACT_SEARCH` | `5` | Search records, not implemented |
| `ACT_SORT` | `6` | Sort operation, planned |
| `ACT_APPLY` | `7` | Apply operation, planned |

## Routing Model

The server decodes the message envelope first. Then routing is based on:

```txt
COM_TYPE + DOMAIN_TYPE + ACTION_TYPE
```

Current request routing:

```txt
REQ + ENTRY + ACT_GET     -> entryRouter get handling
REQ + ENTRY + ACT_CREATE  -> entryRouter create handling
```

Other domains and actions are placeholders at the moment.

## Application Ping

If the server receives a message with:

```txt
COM_TYPE = PING
```

it responds with:

```txt
COM_TYPE     RESP
DOMAIN_TYPE  same as request
ACTION_TYPE  same as request
ID           same as request
PAYLOAD      "PONG"
```

The server currently closes the connection after sending this response.

## Entry GET Request

Current envelope:

```txt
COM_TYPE     REQ
DOMAIN_TYPE  ENTRY
ACTION_TYPE  ACT_GET
```

Current payload:

```txt
ids  VECU32
```

The `ids` vector selects which entries to return.

Special case:

```txt
[0]
```

means "return all entries".

Examples:

```txt
VECU32 [0]       -> all anime entries
VECU32 [1]       -> entry with ID 1
VECU32 [1, 4, 9] -> entries with IDs 1, 4, and 9
```

If an entry ID is not found, the current server catches the error and returns a
`RESP + ENTRY + ACT_GET` message whose payload is a plain text error string.

## Entry GET Response

Current envelope:

```txt
COM_TYPE     RESP
DOMAIN_TYPE  ENTRY
ACTION_TYPE  ACT_GET
ID           same as request
```

Current payload:

```txt
section header
schema
entry record 0
entry record 1
...
entry record N
```

So yes: the current record response already includes schema information before
the records.

### Section Header Payload

```txt
sectionType   U8
title         STR
recordCount   U32
fieldCount    U32
hasSchema     U8
```

Current entry record responses use a database-record section such as:

```txt
sectionType = SECTION_DATABASE_RECORDS
title       = "ANIME_DATA"
fieldCount  = 15
hasSchema   = true
```

`recordCount` is the database section's total record count, not necessarily the
number of records returned by a filtered GET request.

### Schema Payload

```txt
fieldCount  U32

repeat fieldCount times:
  fieldName   STR
  dataType    U8
  filterMode  U8
```

### Schema Data Types

| Name | Value | Meaning |
| --- | ---: | --- |
| `U8` | `0` | Unsigned 8-bit integer |
| `U16` | `1` | Unsigned 16-bit integer |
| `U32` | `2` | Unsigned 32-bit integer |
| `STR` | `3` | String |
| `VECU8` | `4` | Vector of `U8` values |
| `VECU16` | `5` | Vector of `U16` values |
| `UNKNOWN` | `6` | Unknown/invalid |

### Filter Modes

| Name | Value | Meaning |
| --- | ---: | --- |
| `FM_NONE` | `0` | No special filtering |
| `FM_RANGE` | `1` | Range filter |
| `FM_MULTI_VALUE` | `2` | Multi-value filter |
| `FM_RMV` | `3` | Range and multi-value filter |
| `FM_UNKNOWN` | `4` | Unknown/invalid |

### Entry Record Payload

Each entry record is encoded as field values in schema order.

Current entry fields are:

| Field | Type |
| --- | --- |
| `id` | `U32` |
| `rank` | `U32` |
| `title` | `STR` |
| `alt_title` | `STR` |
| `format` | `U8` |
| `status` | `U8` |
| `genres` | `VECU8` |
| `channels` | `VECU8` |
| `audios` | `VECU8` |
| `subtitles` | `VECU8` |
| `fan_service` | `U8` |
| `episode_count` | `U8` |
| `min_runtime` | `U16` |
| `characters` | `VECU16` |
| `ip_group` | `U8` |

The frontend should decode records using the schema included in the response,
not by hard-coding this table forever. The table above documents the current
server behavior.

## Entry CREATE Request

Current envelope:

```txt
COM_TYPE     REQ
DOMAIN_TYPE  ENTRY
ACTION_TYPE  ACT_CREATE
```

Current payload:

```txt
schema
recordCount  U32
entry record 0
entry record 1
...
entry record N
```

The server validates the provided schema and maps incoming fields to the
internal entry metadata before reading the records.

Current success response payload:

```txt
"SUCCESS"
```

Current failure response payload:

```txt
plain text error message
```

## Lookup Tables

Lookup table data exists in the server database, but there is not yet a network
payload for sending lookup tables to the frontend.

The useful frontend shape will eventually be equivalent to:

```js
{
  "genres": ["Action", "Drama", "Romance"],
  "audios": ["Japanese", "English"],
  "subtitles": ["English"]
}
```

A stable binary shape would be:

```txt
tableCount  U32

repeat tableCount times:
  tableName   STR
  valueCount  U32

  repeat valueCount times:
    value  STR
```

This should be sent as a list of named tables instead of a raw unordered map so
the wire format stays deterministic.

## Startup / Boot Messages

The frontend can send startup boot requests to each domain:

```txt
REQ + ENTRY      + ACT_GET + payload target BOOT
REQ + GROUPS     + ACT_GET + payload target BOOT
REQ + CHARACTERS + ACT_GET + payload target BOOT
```

This is not implemented yet, but it is a reasonable direction.

Since entry record responses already include schema, the entry boot flow does
not need to send schema separately unless the frontend wants schema without
records.

For now, it is fine for one boot request to produce multiple response messages,
such as:

```txt
RESP + ENTRY + ACT_GET + lookup tables
RESP + ENTRY + ACT_GET + records with schema
```

If one request produces multiple responses, each response should:

- Reuse the request `ID`.
- Include a payload kind at the start of the payload.
- Be self-describing so the frontend does not depend on response order.

Suggested future payload kind:

```txt
payloadKind  U8
payload      kind-specific bytes
```

Suggested kinds:

| Name | Meaning |
| --- | --- |
| `SCHEMA` | Schema only |
| `LOOKUP_TABLES` | Lookup table labels |
| `RECORDS` | Records, currently section + schema + entries |
| `STATUS` | Non-error status |
| `ERROR` | Structured error |

## Error Handling

Current errors are not structured. Several paths return a normal `RESP`
envelope with a plain text error message as the payload.

This works for debugging, but it means the client has to infer whether a payload
is a normal binary response or an error string.

A future structured error payload should include a payload kind or status code,
for example:

```txt
payloadKind  ERROR
errorCode    U16
message      STR
```

## Compatibility Notes

When changing the envelope or primitive encodings, increment `ENCODE_VERSION`.

When changing only a domain payload, it may be enough to add a domain payload
version or payload kind without changing the whole protocol version.

The current protocol assumes that one decoded WebSocket binary frame contains
one complete Anime Database message.

