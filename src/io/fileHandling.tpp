#pragma once


/*
Parse File Templated Function is designed to parse a file that contains database data in my file format.
The function takes a input of a file path and a class object that is expected to have header, schema, and record handlers.
The function will read a file with the proper format and split it into its Header, Schema and Records
The header provides the database type, the amount of records and the amount of fields each record has.
The schema provides the name and data type for each field in a record
*/
template<typename f> 
void fileHandler::readFile(std::string file, f& handler) {
    std::ifstream source(file);
    if (!source.is_open()) {
        std::cerr << "Failed to open: \t" << file << "\n";
        return;
    }

    schemaData schema;
    fileHeader header{};

    int lineCount = 0;
    std::string line;

    while (getline(source, line)) {

        lineCount++;

        if (lineCount == 1) {
            if (line.empty()) continue;
            header = readHeader(line);
            handler.header(header);
            continue;
        }

        if (lineCount == 2) {
            if (line.empty()) continue;
            schema = readSchema(line, header.fieldCount);
            handler.schema(schema);
            continue;
        }

        if (lineCount >= 4) {
            if (line.empty()) continue;
            recordData record = readRecord(schema, line);
            handler.record(record, schema);
        }
    }
}



