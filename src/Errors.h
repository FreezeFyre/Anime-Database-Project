#include <vector>
#include <string>

#pragma once

struct mapError {
	enum class errorType {
		NO_ID,
		OUT_OF_RANGE,
		BROKEN_DATA
	};

	errorType errortype;
	int id;
	std::string errorMessage;
};

using databaseErrors = std::vector<mapError>;

struct parsingError {
	enum class errorType {
		FILE_NOT_FOUND,
		MISSING_HEADER,
		MISSING_SCHEMA
	};

	errorType errortype;
	int lineNumber;
	std::string errorMessage;
};

using parsingERRORS = std::vector<parsingError>;