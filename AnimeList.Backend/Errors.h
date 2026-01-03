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