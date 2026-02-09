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


using FpResult = enum {
	FP_SUCCESS = 0,
	FP_INCORRECT_FIELD_COUNT = 1,
	FP_INCORRECT_RECORD_COUNT = 2,
	FP_INCORRECT_SCHEMA = 3,
	FP_ERROR_FILE_NOT_FOUND = -1,
	FP_ERROR_INVALID_FILE_TYPE = -2,
	FP_ERROR_INVALID_FIELD_COUNT = -3,
	FP_ERROR_INVALID_RECORD_COUNT = -4,
	FP_ERROR_INVALID_SCHEMA_TYPE = -5,
	FP_ERROR_UNKOWN = -6
};
