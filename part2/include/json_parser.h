#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <string>
#include <vector>

#include "record.h"

// Classic JSON parser for Binance aggregate trades. In this parser we assume the objects for the JSON array
// are the same and have the same fields in the same order. If the order of the fields changes, the parser
// will fail and no error will be returned. In this paerer we parser one by one the characters in the JSON
// string and based on the order we parse the equivalent values.
class JsonParser
{
public:
    JsonParser() = default;
    ~JsonParser() = default;
    JsonParser(const JsonParser &other) = delete;
    JsonParser(JsonParser &&other) = delete;
    JsonParser &operator=(const JsonParser &other) = delete;
    JsonParser &operator=(JsonParser &&other) = delete;

    std::vector<Record> parseRecords(const std::string &json);

private:
    // Skip whitespace characters
    void skipWhitespace(const std::string &s, uint32_t &index) const;

    // Expect a specific character at current position
    bool expectChar(const std::string &s, uint32_t &index, char c) const;

    // Parse a JSON string value
    std::string parseString(const std::string &s, uint32_t &index) const;

    // Parse an integer value
    int64_t parseInt64(const std::string &s, uint32_t &index) const;

    // Parse a boolean value t or f
    bool parseBool(const std::string &s, uint32_t &index) const;

    bool parserFieldInOrder(const std::string &json,
                            uint32_t &index,
                            Record &record,
                            const std::string &fieldNameExpected);

    // Parse a single record object
    Record parseRecord(const std::string &json, uint32_t &index);
};

#endif // JSON_PARSER_H
