add_executable(part2)
target_include_directories(part2 PRIVATE include)
target_sources(part2 PRIVATE src/json_parser.cpp src/json_parser_simd.cpp src/main.cpp)
# Enable AVX2 support for SIMD parser
target_compile_options(part2 PRIVATE -mavx2)


# Find and link libcurl
find_package(CURL REQUIRED)
target_include_directories(part2 PRIVATE ${CURL_INCLUDE_DIRS})
target_link_libraries(part2 PRIVATE ${CURL_LIBRARIES})