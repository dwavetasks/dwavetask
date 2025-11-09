#ifndef RECORD_H
#define RECORD_H

#include <cstdint>
#include <string>
#include <vector>

#include <immintrin.h>

// Binance aggregate trade record
struct Record
{
    int64_t a;     // Aggregate tradeId
    std::string p; // Price
    std::string q; // Quantity
    int64_t f;     // First tradeId
    int64_t l;     // Last tradeId
    int64_t T;     // Timestamp
    bool m;        // Was the buyer the maker?
};

#endif // RECORD_H
