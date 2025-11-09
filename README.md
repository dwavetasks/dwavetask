# Quant Finance Interview Project

A small C++14 project building a Hast Table and a JSON parser.

## Project Structure

```
.
├── CMakeLists.txt               # Main CMake configuration with O3 optimizations
├── README.md
├── part1/                       # Task 1: Hash Table with LRU/MRU
│   ├── CMakeLists.txt
│   ├── include/
│   │   └── hash_table.h         # Hash Table implementation
│   └── src/
│       └── main.cpp             # Test and demonstration code
├── part2/                       # Task 2: JSON Parser
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├─── json_parser.h       # JSON parser implementation
│   │   └── json_parser_simd.h   # SIMD optimised JSON parser
│   └── src/
│       ├── json_parser.cpp      # JSON parser source
│       ├── json_parser.cpp      # SIMD optimised JSON parser source
│       └── main.cpp             # API fetching and benchmarking
└── build/                       # Build output directory
```

## Build Instructions

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build all targets
make

# Run part 1
./part1/part1

# Run part 2
./part2/part2
```

## Part 1

Implementation of a Hash Table together with a double linked list in order to get the Least Recently Used and Most Recently Used items.

The code of the Hash Table exists in [`part1/include/hash_table.h`](part1/include/hash_table.h)

The data is stored in a contiguous `std::array` using unique pointer for the heap. 
The LRU and MRU functionalities are facilitated by the use of a double linked list. If a node is most recently used then it is placed at the beginning of the list. It is first unlinked from its current position and pushed to the front, while keeping sure that the list connections are valid. 
The class also stores member variables for the first and the last node of the double linked list, for easy retrieval and to help for edge cases. 

In the [`main.cpp`](part1/src/main.cpp) file exists code in order to download the book from [https://www.gutenberg.org/files/98/98-0.txt](https://www.gutenberg.org/files/98/98-0.txt). It uses CURL; the words are parsed and stored in a vector of strings.

Then these words are loaded into the Hash Table and [`main.cpp`](part1/src/main.cpp) also has some tests that test basic and edge cases of the Hash Table.


## Part 2

Implementation of a fast JSON parser with data fetching from the Binance API.

For this task I have implemented 2 JSON parsers. The first one is a standard parser that reads data sequentially and the code exists in [`part2/include/json_parser.h`](part2/include/json_parser.h) and the function definition is in [`part2/src/json_parser.cpp`](part2/src/json_parser.cpp). Then I implemented a second parser that incorporates Single Instruction Multiple Data (SIMD) using AVX2 vectorized instructions in order to speed up the computations. The second parser instead of checking one by one the characters of the stringified JSON, it checks in parallel 32 bytes of characters in the string using AVX2 registers. In this process the `"` quote locations of the stringified JSON are found and having those locations the rest of the implementation can faster determine the positions of the values of the JSON. The speedup is due to SIMD where a single instruction of checking if a character is equal to `"` can be parallelized to multiple data at every time. The implementation of the SIMD JSON parser can be found here [`part2/include/json_parser_simd.h`](part2/include/json_parser_simd.h) and here [`part2/src/json_parser_simd.cpp`](part2/src/json_parser_simd.cpp).

Regarding the time complexity, for both parsing algorithms for a specific JSON object (and not the whole array), the time complexity is constant and thus it involves a constant amount of operations, thus it is O(1). However in the faster version of the SIMD JSON parser while we still need a constant amount of operations to parse a single JSON object, we execute a couple of them in parallel and this does not affect the per instruction performance. For this reason we execute fewer instructions since their replication happens with no cost and the complexity can be reduced to O(1/m) where m is the factor of reduced instructions from SIMD.

It is important that both implementations expect this schema of JSON to be inserted otherwise the parsing will fail unexpectedly:

```json
[
  {
    "a": 26129,             // Aggregate tradeId
    "p": "0.01633102",      // Price
    "q": "4.70443515",      // Quantity
    "f": 27781,             // First tradeId
    "l": 27781,             // Last tradeId
    "T": 1498793709153,     // Timestamp
    "m": true               // Was the buyer the maker?
  }
]
```

In [`src/main.cpp`](part2/src/main.cpp) these 2 implementations are benchmarked. First, data from the Binance API is fetched using CURL and that returns a string of a stringified JSON ready to be parsed. 
We use both of our parsers and we parse the same string in a loop of around 100,000 times, so as to have a more accurate benchmark time, due to scheduling, caching, etc. Then we find the average time of parsing a single JSON object.

We measure significant improvement using SIMD over the classic implementation. We can achieve up to 3x times better performance. 
On an i7 12th gen with `-O3` flag enabled, the classic JSON parser needs around **450ns** per entry while the SIMD parser needs **140ns**!

## Requirements

- CMake 3.10 or higher
- clang-tidy
- libcurl