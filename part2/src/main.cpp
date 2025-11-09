#include <chrono>
#include <iostream>
#include <string>

#include <curl/curl.h>

#include "json_parser.h"
#include "json_parser_simd.h"
#include "record.h"

// Callback for libcurl to write received data into a std::string
static uint32_t write_callback(char *ptr, uint32_t size, uint32_t nmemb, void *userdata)
{
    std::string *buffer = static_cast<std::string *>(userdata);
    buffer->append(ptr, size * nmemb);
    return size * nmemb;
}

std::string download_json(const std::string &url)
{
    std::string content;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL *curl = curl_easy_init();

    if (curl != nullptr)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        // Check if easy perform was successful
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            std::cerr << "Fetching data failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    return content;
}

int main()
{
    // Binance Futures endpoint
    const std::string symbol = "BTCUSDT";
    const std::string limit = "10";

    const std::string url = "https://fapi.binance.com/fapi/v1/aggTrades?symbol=" + symbol + "&limit=" + limit;

    std::cout << "Downloading trade data\n";
    std::string jsonData = download_json(url);

    // Measure parsing time with multiple iterations for better benchmark accuracy
    const uint32_t iterations = 100000;

    // ===========================================================================
    // ==================== CLASSIC PARSER BENCHMARK =============================
    // ===========================================================================
    std::cout << "\n========== CLASSIC PARSER BENCHMARK ==========\n"
              << std::endl;

    JsonParser parser;
    auto startTimeClassic = std::chrono::high_resolution_clock::now();
    std::vector<Record> trades;
    for (uint32_t i = 0; i < iterations; ++i)
    {
        trades = parser.parseRecords(jsonData);
    }

    auto endTimeClassic = std::chrono::high_resolution_clock::now();
    auto durationClassic = std::chrono::duration_cast<std::chrono::nanoseconds>(endTimeClassic -
                                                                                startTimeClassic);
    // Calculate time per record for classic parser
    const uint32_t totalRecordsClassic = trades.size() * iterations;
    const double averageTimePerRecordClassic = static_cast<double>(durationClassic.count()) /
                                               static_cast<double>(totalRecordsClassic);

    std::cout << "Parsed first trade:\n"
              << std::endl;

    for (const auto &trade : trades)
    {
        std::cout << "Trade ID: " << trade.a << "\n";
        std::cout << "  Price: " << trade.p << "\n";
        std::cout << "  Quantity: " << trade.q << "\n";
        std::cout << "  First Trade ID: " << trade.f << "\n";
        std::cout << "  Last Trade ID: " << trade.l << "\n";
        std::cout << "  Timestamp: " << trade.T << "\n";
        std::cout << "  Buyer is maker: " << (trade.m ? "true" : "false") << "\n";
        std::cout << std::endl;
        break; // Print only the first trade
    }

    // Display timing information for classic parser
    std::cout << "\n=== CLASSIC PARSER Performance Metrics ===" << std::endl;
    std::cout << "Total records parsed: " << totalRecordsClassic << std::endl;
    std::cout << "Total time: " << durationClassic.count() << " nanoseconds" << std::endl;
    std::cout << "Average time per record: " << averageTimePerRecordClassic << " nanoseconds" << std::endl;
    std::cout << "Average time per record: " << (averageTimePerRecordClassic / 1000.0) << " microseconds"
              << std::endl;

    // ===========================================================================
    // ======================= SIMD PARSER BENCHMARK =============================
    // ===========================================================================
    std::cout << "\n\n========== SIMD PARSER BENCHMARK ==========\n"
              << std::endl;

    JsonParserSIMD parserSIMD(limit.empty() ? 5 : std::stoul(limit));

    auto startTimeSIMD = std::chrono::high_resolution_clock::now();
    std::vector<Record> tradesSIMD;
    for (uint32_t i = 0; i < iterations; ++i)
    {
        tradesSIMD = parserSIMD.parseRecords(jsonData);
    }
    auto endTimeSIMD = std::chrono::high_resolution_clock::now();
    auto durationSIMD = std::chrono::duration_cast<std::chrono::nanoseconds>(endTimeSIMD - startTimeSIMD);

    // Calculate time per record for SIMD parser
    const uint32_t totalRecordsSIMD = tradesSIMD.size() * iterations;
    const double averageTimePerRecordSIMD = static_cast<double>(durationSIMD.count()) /
                                            static_cast<double>(totalRecordsSIMD);

    std::cout << "Parsed first trade:\n"
              << std::endl;

    for (const auto &trade : tradesSIMD)
    {
        std::cout << "Trade ID: " << trade.a << "\n";
        std::cout << "  Price: " << trade.p << "\n";
        std::cout << "  Quantity: " << trade.q << "\n";
        std::cout << "  First Trade ID: " << trade.f << "\n";
        std::cout << "  Last Trade ID: " << trade.l << "\n";
        std::cout << "  Timestamp: " << trade.T << "\n";
        std::cout << "  Buyer is maker: " << (trade.m ? "true" : "false") << "\n";
        std::cout << std::endl;
        break; // Print only the first trade
    }

    // Display timing information for SIMD parser
    std::cout << "\n=== SIMD PARSER Performance Metrics ===" << std::endl;
    std::cout << "Total records parsed: " << totalRecordsSIMD << std::endl;
    std::cout << "Total time: " << durationSIMD.count() << " nanoseconds" << std::endl;
    std::cout << "Average time per record: " << averageTimePerRecordSIMD << " nanoseconds" << std::endl;
    std::cout << "Average time per record: " << (averageTimePerRecordSIMD / 1000.0) << " microseconds"
              << std::endl;

    // ==================== PERFORMANCE COMPARISON ====================
    std::cout << "\n\n========== PERFORMANCE COMPARISON ==========\n"
              << std::endl;
    const double speedup = averageTimePerRecordClassic / averageTimePerRecordSIMD;
    const double percentImprovement =
        ((averageTimePerRecordClassic - averageTimePerRecordSIMD) / averageTimePerRecordClassic) * 100.0;

    std::cout << "Classic Parser: " << averageTimePerRecordClassic << " ns/record" << std::endl;
    std::cout << "SIMD Parser:    " << averageTimePerRecordSIMD << " ns/record" << std::endl;
    std::cout << "Speedup:        " << speedup << "x faster" << std::endl;
    std::cout << "Improvement:    " << percentImprovement << "%" << std::endl;

    return 0;
}
