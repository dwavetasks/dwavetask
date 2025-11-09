#include "json_parser_simd.h"

std::vector<Record> JsonParserSIMD::parseRecords(const std::string &json)
{
    std::vector<Record> records;

    indexesOfQuotesOnJsonString.clear();

    // Find in parallel of 32 byte chunks all quotes in the JSON string
    find_quotes_avx2(json);

    // There are 18 quote characters in the json example above
    // to help us verify correctness
    // We can separate the quotes of the keys from the quotes of string values.
    // We know that the first key has 2 quotes, then each key has 2 quotes
    // However the p and q keys have string values which add 2 extra quotes each
    // So the pattern of quotes for keys only is:
    // 0: "a" -> 2 quotes
    // 1: "p" -> 4 quotes (2 for key, 2 for string value)
    // 2: "q" -> 4 quotes (2 for key, 2 for string value)
    // 3: "f" -> 2 quotes
    // 4: "l" -> 2 quotes
    // 5: "T" -> 2 quotes
    // 6: "m" -> 2 quotes
    // Total: 18 quotes

    // Number of keys
    const uint32_t correctOrderKeysSize = 7;

    // Process everything sequentially
    uint32_t jsonQuoteCount = 0;
    uint32_t correctOffsetForJsonKeys = 2; // Start with 2 then change to 4 for p and q later
    uint32_t expectedKeyIndex = 0;
    uint32_t indexOnJsonString = 0;

    Record record{};
    bool recordStarted = false;

    for (uint32_t i = 0; i < indexesOfQuotesOnJsonString.size();)
    {
        jsonQuoteCount = i % jsonQuotes;
        expectedKeyIndex = expectedKeyIndex % correctOrderKeysSize;
        indexOnJsonString = indexesOfQuotesOnJsonString[i]; // get current quote index

        if (jsonQuoteCount >= 2 && jsonQuoteCount <= 6)
        {
            // p and q keys have string values adding 2 extra quotes
            correctOffsetForJsonKeys = 4;
        }
        else
        {
            correctOffsetForJsonKeys = 2;
        }

        // Get the values of every key
        if (expectedKeyIndex == 0)
        {
            if (recordStarted)
            {
                records.push_back(record);
            }
            record = Record{};
            recordStarted = true;
            // process "a" key
            // a value starts at current indexOnJsonString + 4
            // a value ends at the -1 before the next quote index
            const uint32_t indexOnJsonStringStart = indexOnJsonString + 4;
            const uint32_t indexOnJsonStringEnd = indexesOfQuotesOnJsonString[i + correctOffsetForJsonKeys] -
                                                  1;
            std::string a_value = json.substr(indexOnJsonStringStart,
                                              indexOnJsonStringEnd - indexOnJsonStringStart);
            record.a = parseInt64FromString(a_value);
        }
        else if (expectedKeyIndex == 1)
        {
            // process "p" key
            // p value starts at current indexOnJsonString + 4
            // p value ends at the -1 before the next quote index
            const uint32_t indexOnJsonStringStart = indexOnJsonString + 4;
            const uint32_t indexOnJsonStringEnd = indexesOfQuotesOnJsonString[i + correctOffsetForJsonKeys] -
                                                  1;
            std::string p_value = json.substr(indexOnJsonStringStart,
                                              indexOnJsonStringEnd - indexOnJsonStringStart);
            record.p = p_value;
        }
        else if (expectedKeyIndex == 2)
        {
            // process "q" key
            // q value starts at current indexOnJsonString + 4
            // q value ends at the -1 before the next quote index
            const uint32_t indexOnJsonStringStart = indexOnJsonString + 4;
            const uint32_t indexOnJsonStringEnd = indexesOfQuotesOnJsonString[i + correctOffsetForJsonKeys] -
                                                  1;
            std::string q_value = json.substr(indexOnJsonStringStart,
                                              indexOnJsonStringEnd - indexOnJsonStringStart);
            record.q = q_value;
        }
        else if (expectedKeyIndex == 3)
        {
            // process "f" key
            // f value starts at current indexOnJsonString + 4
            // f value ends at the -1 before the next quote index
            const uint32_t indexOnJsonStringStart = indexOnJsonString + 4;
            const uint32_t indexOnJsonStringEnd = indexesOfQuotesOnJsonString[i + correctOffsetForJsonKeys] -
                                                  1;
            std::string f_value = json.substr(indexOnJsonStringStart,
                                              indexOnJsonStringEnd - indexOnJsonStringStart);
            record.f = parseInt64FromString(f_value);
        }
        else if (expectedKeyIndex == 4)
        {
            // process "l" key
            // l value starts at current indexOnJsonString + 4
            // l value ends at the -1 before the next quote index
            const uint32_t indexOnJsonStringStart = indexOnJsonString + 4;
            const uint32_t indexOnJsonStringEnd = indexesOfQuotesOnJsonString[i + correctOffsetForJsonKeys] -
                                                  1;
            std::string l_value = json.substr(indexOnJsonStringStart,
                                              indexOnJsonStringEnd - indexOnJsonStringStart);
            record.l = parseInt64FromString(l_value);
        }
        else if (expectedKeyIndex == 5)
        {
            // process "T" key
            // T value starts at current indexOnJsonString + 4
            // T value ends at the -1 before the next quote index
            const uint32_t indexOnJsonStringStart = indexOnJsonString + 4;
            const uint32_t indexOnJsonStringEnd = indexesOfQuotesOnJsonString[i + correctOffsetForJsonKeys] -
                                                  1;
            std::string T_value = json.substr(indexOnJsonStringStart,
                                              indexOnJsonStringEnd - indexOnJsonStringStart);
            record.T = parseInt64FromString(T_value);
        }
        else if (expectedKeyIndex == 6)
        {
            // process "m" key
            // m value starts at current indexOnJsonString + 4
            // we do not care about the ending quote since boolean is t or f
            const uint32_t indexOnJsonStringStart = indexOnJsonString + 4;
            std::string m_value = json.substr(indexOnJsonStringStart, 1);
            record.m = parseBoolFromString(m_value);
        }

        i = i + correctOffsetForJsonKeys;
        ++expectedKeyIndex;
    }

    if (recordStarted)
    {
        records.push_back(record);
    }

    return records;
}

void JsonParserSIMD::find_quotes_avx2(const std::string &s)
{
    const uint32_t n = s.size();
    uint32_t i = 0;

    // AVX2 register with 32 quote characters [", ", ", ..., "]
    const __m256i quotesVectorized = _mm256_set1_epi8('"');

    // Process 32 bytes at a time
    for (; i + 32 <= n; i += 32)
    {
        // Cast string to __m256i pointer and load 32 bytes
        __m256i chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(s.data() + i));

        __m256i quotesMaskVectorized256 = _mm256_cmpeq_epi8(chunk, quotesVectorized);

        // Turn bits of the 256 but quotesMaskVectorized256 into bytes in a 32-bit mask
        uint32_t quotesMaskVectorized32 = static_cast<uint32_t>(
            _mm256_movemask_epi8(quotesMaskVectorized256));

        // Extract bit positions each '1' corresponds to a quote character
        while (quotesMaskVectorized32 != 0)
        {
            // index of least significant 1 bit because x86 is little-endian
            uint32_t bit_index = __builtin_ctz(quotesMaskVectorized32);
            indexesOfQuotesOnJsonString.push_back(i + bit_index);
            // clear least significant 1 bit
            quotesMaskVectorized32 &= quotesMaskVectorized32 - 1;
        }
    }

    // Tail remaining bytes from the string.size() % 32 remainder will be processed once here
    for (; i < n; ++i)
    {
        char c = s[i];
        if (c == '"')
        {
            indexesOfQuotesOnJsonString.push_back(i);
        }
    }
}

int64_t JsonParserSIMD::parseInt64FromString(const std::string &s) const
{
    if (s.empty())
    {
        return 0;
    }

    uint32_t index = 0;
    bool negative = false;
    if (s[index] == '-')
    {
        negative = true;
        ++index;
    }

    int64_t value = 0;
    while (index < s.size() && s[index] >= '0' && s[index] <= '9')
    {
        value = value * 10 + (s[index] - '0');
        ++index;
    }

    return negative ? -value : value;
}

bool JsonParserSIMD::parseBoolFromString(const std::string &s) const
{
    if (s.empty())
    {
        return false;
    }
    if (s[0] == 't')
    {
        return true;
    }
    else if (s[0] == 'f')
    {
        return false;
    }
    return false;
}
