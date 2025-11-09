#ifndef JSON_PARSER_SIMD_H
#define JSON_PARSER_SIMD_H

#include <cstdint>
#include <string>
#include <vector>

#include <immintrin.h>

#include "record.h"

// Optimized JSON parser for Binance aggregate trades using SIMD (AVX2). In this parser we assume the objects
// for the JSON array are the same and have the same fields in the same order. If the order of the fields
// changes, the parser will fail and no error will be returned. In this parser we use AVX2 to speed up the
// JSON parsing by locating fast and in parallel the quotes of the JSON string values.
// For this JSON object knowing the qutores location is enough to extract all the values of the fields.
//
// How parsing works:
// 1. Use AVX2 to find all quote characters in the JSON string and store their indices in vector. We also
// reserve vector capacity to avoid reallocations.
//
// 2. We know in our specific case that each record has 18 quotes.
// 0: "a" -> 2 quotes
// 1: "p" -> 4 quotes (2 for key, 2 for string value)
// 2: "q" -> 4 quotes (2 for key, 2 for string value)
// 3: "f" -> 2 quotes
// 4: "l" -> 2 quotes
// 5: "T" -> 2 quotes
// 6: "m" -> 2 quotes
// Since we know this pattern we can excract the values directly from the JSON string if we skip 4 characters
// from the first quote. Then in order to jump to the next first quote of the next key we need to skip either
// 2 or 4 quotes depending on the key. If we are at p or q keys we have to skip 4 quotes, otherwise we skip 2
// quotes.
//
// 3. Fianly ee perfom this process sequentially for all records in the JSON string and then iteratively for
// all the JSON objcets in the array.
class JsonParserSIMD
{
public:
    explicit JsonParserSIMD(uint32_t expectedRecordCount)
    {
        indexesOfQuotesOnJsonString.reserve(expectedRecordCount * jsonQuotes);
    }
    ~JsonParserSIMD() = default;
    JsonParserSIMD(const JsonParserSIMD &other) = delete;
    JsonParserSIMD(JsonParserSIMD &&other) = delete;
    JsonParserSIMD &operator=(const JsonParserSIMD &other) = delete;
    JsonParserSIMD &operator=(JsonParserSIMD &&other) = delete;

    // Constant expression for quotes per record
    static constexpr uint32_t jsonQuotes = 18;

    // Pasrsee records from JSON string in the patter of using SIMD to find quotes. There are 18 quotes on the
    // JSON and the skipping pattern is known.
    std::vector<Record> parseRecords(const std::string &json);

private:
    // Return indices of quotes using vectorized AVX2 registers and instructions.
    void find_quotes_avx2(const std::string &s);

    // Parse an integer value from string
    int64_t parseInt64FromString(const std::string &s) const;

    // Parse a boolean value from string t or f
    bool parseBoolFromString(const std::string &s) const;

    std::vector<uint32_t> indexesOfQuotesOnJsonString;
};

#endif // JSON_PARSER_SIMD_H
