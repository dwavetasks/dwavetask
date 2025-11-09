#include "json_parser.h"

std::vector<Record> JsonParser::parseRecords(const std::string &json)
{
    std::vector<Record> records;
    uint32_t index = 0;

    skipWhitespace(json, index);
    if (!expectChar(json, index, '['))
    {
        return records;
    }

    skipWhitespace(json, index);
    if (index < json.size() && json[index] == ']')
    {
        // Empty array
        ++index;
        return records;
    }

    // Parse array elements
    while (true)
    {
        Record record = parseRecord(json, index);
        records.push_back(record);

        // Check for comma or end of array
        skipWhitespace(json, index);
        if (index < json.size() && json[index] == ',')
        {
            ++index;
            continue;
        }
        else if (index < json.size() && json[index] == ']')
        {
            ++index;
            break;
        }
        else
        {
            // Error just return what we have
            break;
        }
    }

    return records;
}

// Skip whitespace characters
void JsonParser::skipWhitespace(const std::string &s, uint32_t &index) const
{
    while (index < s.size() && std::isspace(static_cast<unsigned char>(s[index])) != 0)
    {
        ++index;
    }
}

// Expect a specific character at current position
bool JsonParser::expectChar(const std::string &s, uint32_t &index, char c) const
{
    skipWhitespace(s, index);
    if (index >= s.size() || s[index] != c)
    {
        return false;
    }
    ++index;
    return true;
}

// Parse a JSON string value
std::string JsonParser::parseString(const std::string &s, uint32_t &index) const
{
    skipWhitespace(s, index);
    if (index >= s.size() || s[index] != '"')
    {
        return "";
    }
    ++index;

    std::string result;
    while (index < s.size() && s[index] != '"')
    {
        result.push_back(s[index]);
        ++index;
    }

    if (index >= s.size() || s[index] != '"')
    {
        return result; // Return what we have
    }
    ++index;

    return result;
}

// Parse an integer value
int64_t JsonParser::parseInt64(const std::string &s, uint32_t &index) const
{
    skipWhitespace(s, index);
    if (index >= s.size())
    {
        return 0;
    }

    bool negative = false;
    if (s[index] == '-')
    {
        negative = true;
        ++index;
    }

    if (index >= s.size() || std::isdigit(static_cast<unsigned char>(s[index])) == 0)
    {
        return 0;
    }

    int64_t value = 0;
    while (index < s.size() && std::isdigit(static_cast<unsigned char>(s[index])) != 0)
    {
        value = value * 10 + (s[index] - '0');
        ++index;
    }

    return negative ? -value : value;
}

// Parse a boolean value t or f
bool JsonParser::parseBool(const std::string &s, uint32_t &index) const
{
    skipWhitespace(s, index);
    if (s.compare(index, 1, "t") == 0)
    {
        index += 4;
        return true;
    }
    else if (s.compare(index, 1, "f") == 0)
    {
        index += 5;
        return false;
    }
    return false;
}

bool JsonParser::parserFieldInOrder(const std::string &json,
                                    uint32_t &index,
                                    Record &record,
                                    const std::string &fieldNameExpected)
{
    // Parse expected field
    skipWhitespace(json, index);

    std::string fieldName = parseString(json, index);

    if (!expectChar(json, index, ':'))
    {
        return false;
    }
    if (fieldName == "a" && fieldNameExpected == "a")
    {
        record.a = parseInt64(json, index);
    }
    else if (fieldName == "p" && fieldNameExpected == "p")
    {
        record.p = parseString(json, index);
    }
    else if (fieldName == "q" && fieldNameExpected == "q")
    {
        record.q = parseString(json, index);
    }
    else if (fieldName == "f" && fieldNameExpected == "f")
    {
        record.f = parseInt64(json, index);
    }
    else if (fieldName == "l" && fieldNameExpected == "l")
    {
        record.l = parseInt64(json, index);
    }
    else if (fieldName == "T" && fieldNameExpected == "T")
    {
        record.T = parseInt64(json, index);
    }
    else if (fieldName == "m" && fieldNameExpected == "m")
    {
        record.m = parseBool(json, index);
    }
    else
    {
        return false;
    }

    // Check for comma or end of the field
    skipWhitespace(json, index);
    if (index < json.size() && json[index] == ',')
    {
        ++index;
        return true;
    }
    else
    {
        return false;
    }
}

// Parse a single record object
Record JsonParser::parseRecord(const std::string &json, uint32_t &index)
{
    skipWhitespace(json, index);
    if (!expectChar(json, index, '{'))
    {
        return Record{};
    }

    Record record{};

    // Parse field names in order a p q f l T m
    parserFieldInOrder(json, index, record, "a");
    parserFieldInOrder(json, index, record, "p");
    parserFieldInOrder(json, index, record, "q");
    parserFieldInOrder(json, index, record, "f");
    parserFieldInOrder(json, index, record, "l");
    parserFieldInOrder(json, index, record, "T");
    parserFieldInOrder(json, index, record, "m");

    // Check for end of object
    if (index < json.size() && json[index] == '}')
    {
        ++index;
        return record;
    }

    return record;
}
