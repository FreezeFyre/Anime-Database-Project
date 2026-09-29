#pragma once


enum class ValidationScope : uint8_t {
	ALL, // Checks all validation scopes
	ALL_LOCAL,
	IDS, // checks exclusivness of IDs and index tables
	RANKS, // checks continuity of ranks
	TITLES, // checks exclusiveness of titles
	RANGES, // checks that scalar fields are within valid ranges
	REFERENECES, // checks that fields exist or are in range of their reference
	EXTERNAL_REFERENCES // Used with overloaded func. ex(check that entry group is in groupDB)
};

// not sure how to best approach external references so im not hardcoding what field to look at...
