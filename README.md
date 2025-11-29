# AnimeList.Backend

A modern C++ backend for managing anime metadata, filtering, sorting, and server-based client handling. Designed to be fast, maintainable, and fully structured for integration with a future web frontend.

---

## Overview

AnimeList.Backend is a C++-only backend that provides:

- Console mode for local filtering, debugging, and data inspection.
- Server mode for multi-client filtering, sorting, and query processing.
- An in-memory database using `std::unordered_map` for constant-time lookup.
- A structured metadata system for anime entries and franchise groups.
- Planned character database with bidirectional relationships.
- A grouping system for franchises/series similar to MAL and AniList.

This backend is intended to support a future website and API for browsing, filtering, and exploring anime data.

---

# Modes of Operation

## 1. Console Mode

When launched, the program prompts the user:

```
Run as:
1. Console Only
2. Server
```

Console mode allows:

- Loading filter definitions from JSON files.
- Applying sorting and filtering locally.
- Outputting results directly to the console.
- (Future) Interactive filter-builder tool to construct filter JSON files.

---

## 2. Server Mode

When launched as a server, the program listens for incoming client connections.

### Connection Logging
For each connected client, the server logs:
- IP address  
- Connection and disconnection time  
- Ping estimate  

### Request Processing Logs
For each processed request, the server logs:
- Filters received from the client  
- Sorting method used  
- Number of results and their IDs  
- The thread handling the request  
- Start and end timestamps  

### Planned Command Interface
- View active clients  
- View filter history per client  
- View thread load  
- Toggle "client-only" mode for debugging  
- General administrative diagnostics  

---

# Filter Input Flow

Filters follow this order:

1. Includes  
2. Excludes  
3. Ranges  

For each section, if enabled:
- The system prompts for the metadata category (example: genres, services, dubs, etc.)
- The user enters values as vectors (one per line)
- Input is terminated by a newline marker (`/n`)

The frontend will eventually generate these JSON structures automatically.

---

# Database Architecture

Anime entries are stored in:

```cpp
std::unordered_map<int, Anime> anime_db;
```

- **Key:** integer ID  
- **Value:** `Anime` struct  

This allows:
- Constant-time lookup
- Fast filtering and sorting operations
- Stable references to entries for server responses

---

# Anime Entry Structure

Each anime entry contains the following fields:

### Name
Primary English or marketed title.

### Alternate Name
Japanese title written in romaji.

### Rank
Personal ranking.  
`0` indicates unranked.

### Genres
Five genre groups are planned:
- World  
- Story  
- Other  
- (Two additional groups to be defined)  

Stored as integer IDs.

### Status
One of the following:
- Unreleased  
- In-Progress  
- Limbo  
- Cancelled  
- Completed  

### Services
Streaming platforms where the anime is available.

### Audio Languages
Common languages include:
- English  
- Japanese  
- Chinese  

### Subtitle Languages
Corresponds to available subtitle languages.

### Fan Service Rating (0–10)
Indicates the severity and prominence of fan service content.

### Episode Count  
Total number of episodes.

### Average Episode Length  
In minutes.

### Total Runtime  
Computed in minutes.

---

# Future Fields

### Character IDs
Anime entries will store character IDs.  
Characters will store anime IDs.  
Character entries will be considered the source of truth.

### Group ID
Identifies the franchise or series the entry belongs to.  
For example:

```
Group: Sword Art Online
- SAO Season 1
- SAO Season 2
- Alicization
- Ordinal Scale
- Progressive films
```

### Art / Animation Rating (0–10)
Measures animation quality, consistency, detail, and overall visual execution.

---

# Group / Franchise Structure

Groups are stored in:

```cpp
std::unordered_map<int, Group> group_db;
```

Each group contains:

- Title  
- Alternate Title  
- Season Count  
- Movie Count  
- Total Runtime  
- Entry IDs belonging to the group  

Season and movie counts are generated from the set of associated entries.

---

# Fan Service Rating Scale (0–10)

0 — No fan service  
1 — Minimal  
2 — Very mild  
3 — Light  
4 — Noticeable  
5 — Moderate  
6 — High  
7 — Very high  
8 — Extreme  
9 — Borderline explicit  
10 — Explicit fanservice emphasis  

---

# Art / Animation Quality Scale (0–10)

0 — Nonfunctional  
1 — Very poor  
2 — Poor  
3 — Below average  
4 — Average  
5 — Decent  
6 — Good  
7 — Very good  
8 — Great  
9 — Exceptional  
10 — Industry-leading  

---

# Future Improvements

Planned improvements include:

- Build instructions and CMake configuration  
- REST API documentation for server mode  
- Data schema documentation  
- Architecture diagrams  
- Frontend integration notes  

This backend is evolving as additional metadata and system features are planned.
