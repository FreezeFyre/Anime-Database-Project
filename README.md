# AnimeList Backend

This is a simple command line tool for storing and viewing anime metadata.
The project uses a custom tokenizer to parse a pseudo-JSON file.

## Building

The project is a single `main.cpp` file. You can build it using `g++`:

```bash
 g++ AnimeList.Backend./main.cpp -std=c++17 -o anime
```

## Running

Run the executable and optionally provide the path to your data file:

```bash
 ./anime path/to/anime_data.json
```

If no path is provided, it defaults to `AnimeList.Backend./anime_data.json`.

## Data Format

The data file is not strict JSON. Each line inside the surrounding array
represents one anime entry and is parsed with a manual tokenizer.
