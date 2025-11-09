#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <curl/curl.h>

#include "hash_table.h"

static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    std::string *buffer = static_cast<std::string *>(userdata);
    buffer->append(ptr, size * nmemb);
    return size * nmemb;
}

std::vector<std::string> download_and_parse_words(const std::string &url)
{
    std::string content;
    std::vector<std::string> words;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL *curl = curl_easy_init();

    if (curl != nullptr)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content);
        // Check if easy perform was successful
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            std::cerr << "Fetching book data failed" << std::endl;
        }

        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    std::istringstream stream(content);
    std::string word;
    while (stream >> word)
    {
        words.push_back(word);
    }

    return words;
}

int main()
{
    std::cout << "Downloading book\n";
    std::vector<std::string> words = download_and_parse_words("https://www.gutenberg.org/files/98/98-0.txt");

    for (size_t i = 0; i < 10 && i < words.size(); ++i)
    {
        std::cout << words[i] << "\n";
    }

    std::cout << "\nTotal words: " << words.size() << "\n";

    const uint32_t tableSize = 20000;

    HashTable<tableSize> bookHashTable;

    for (const auto &word : words)
    {
        auto getResult = bookHashTable.get(word);
        const bool sucess = std::get<0>(getResult);
        if (sucess)
        {
            const uint32_t currentCount = std::get<1>(getResult);
            const bool insertSucess = bookHashTable.insert(word, currentCount + 1);
            if (!insertSucess)
            {
                std::cerr << "Failed to update count for word: " << word << "\n";
            }
        }
        else
        {
            const bool insertSucess = bookHashTable.insert(word, 1);
            if (!insertSucess)
            {
                std::cerr << "Failed to update count for word: " << word << "\n";
            }
        }
    }

    // Test the book hash table
    // Get the count for some test words
    std::vector<std::string> sampleWords = {"the", "a",   "12",   "Gutenberg", "to",  "unprecedented",
                                            "of",  "and", "city", "1231231",   "Bob", "City"};
    for (const auto &sampleWord : sampleWords)
    {
        auto result = bookHashTable.get(sampleWord);
        if (std::get<0>(result))
        {
            std::cout << "Word: '" << sampleWord << "' Count: " << std::get<1>(result) << "\n";
        }
        else
        {
            std::cout << "Word: '" << sampleWord << "' not found in hash table.\n";
        }
    }

    // Tests for hash table //

    // Test basic insert, get, remove
    HashTable<5> testTable;
    testTable.insert("aa", 1);
    testTable.insert("bb", 2);
    testTable.insert("cc", 3);
    const auto get1 = testTable.get("aa");
    if (!(std::get<0>(get1) && std::get<1>(get1) == 1))
    {
        std::cout << "Error in get" << std::endl;
    }

    testTable.remove("bb");
    const auto get2 = testTable.get("bb");
    ;
    if (std::get<0>(get2))
    {
        std::cout << "Error in remove" << std::endl;
    }

    // Add again bb
    testTable.insert("bb", 20);
    const auto get3 = testTable.get("bb");
    ;
    if (!(std::get<0>(get3) && std::get<1>(get3) == 20))
    {
        std::cout << "Error in insert after remove" << std::endl;
    }

    // Check LRU order
    const auto first = testTable.get_first();
    // cc must be the last item since aa was accessed first
    if (!(std::get<0>(first) && std::get<0>(std::get<1>(first)) == "cc"))
    {
        std::cout << "Error in get_first" << std::endl;
    }
    const auto last = testTable.get_last();
    ;
    if (!(std::get<0>(last) && std::get<0>(std::get<1>(last)) == "bb"))
    {
        std::cout << "Error in get_last" << std::endl;
    }

    // If removing cc then aa must become first and still bb last
    testTable.remove("cc");
    const auto firstAA = testTable.get_first();
    if (!(std::get<0>(firstAA) && std::get<0>(std::get<1>(firstAA)) == "aa"))
    {
        std::cout << "Error in get_first after remove" << std::endl;
    }
    const auto lastBB = testTable.get_last();
    if (!(std::get<0>(lastBB) && std::get<0>(std::get<1>(lastBB)) == "bb"))
    {
        std::cout << "Error in get_last after remove" << std::endl;
    }

    // New table to test full capacity
    HashTable<3> smallTable;
    smallTable.insert("one", 1);
    smallTable.insert("two", 2);
    smallTable.insert("three", 3);
    const bool insertFail = smallTable.insert("four", 4);
    if (insertFail)
    {
        std::cout << "Error in insert full table" << std::endl;
    }
    // Remove one then insert must work
    smallTable.remove("three");
    const bool insertSucess = smallTable.insert("four", 4);
    if (!insertSucess)
    {
        std::cout << "Error in insert full table" << std::endl;
    }
    // Removing a non existing key must fail
    const bool removeFail = smallTable.remove("three");
    if (removeFail)
    {
        std::cout << "Error in remove non existing key" << std::endl;
    }

    // Last mast be one and first two
    const auto firstOne = smallTable.get_first();
    if (!(std::get<0>(firstOne) && std::get<0>(std::get<1>(firstOne)) == "one"))
    {
        std::cout << "Error in get_first after insert in full table" << std::endl;
    }
    const auto lastFour = smallTable.get_last();
    if (!(std::get<0>(lastFour) && std::get<0>(std::get<1>(lastFour)) == "four"))
    {
        std::cout << "Error in get_last after insert in full table" << std::endl;
    }

    // Edge case tests of empty table and get_first/get_last
    HashTable<10> emptyTable;
    const auto emptyFirst = emptyTable.get_first();
    if (std::get<0>(emptyFirst))
    {
        std::cout << "Error in get_first on empty table" << std::endl;
    }
    const auto emptyLast = emptyTable.get_last();
    if (std::get<0>(emptyLast))
    {
        std::cout << "Error in get_last on empty table" << std::endl;
    }
    // Add one element and check first and last
    emptyTable.insert("only", 42);
    const auto onlyFirst = emptyTable.get_first();
    if (!(std::get<0>(onlyFirst) && std::get<0>(std::get<1>(onlyFirst)) == "only"))
    {
        std::cout << "Error in get_first on single element table" << std::endl;
    }
    const auto onlyLast = emptyTable.get_last();
    if (!(std::get<0>(onlyLast) && std::get<0>(std::get<1>(onlyLast)) == "only"))
    {
        std::cout << "Error in get_last on single element table" << std::endl;
    }
    // Remove the only element and check again
    emptyTable.remove("only");
    const auto emptyFirstAgain = emptyTable.get_first();
    if (std::get<0>(emptyFirstAgain))
    {
        std::cout << "Error in get_first on empty table after removing only element" << std::endl;
    }
    const auto emptyLastAgain = emptyTable.get_last();
    if (std::get<0>(emptyLastAgain))
    {
        std::cout << "Error in get_last on empty table after removing only element" << std::endl;
    }

    return 0;
}