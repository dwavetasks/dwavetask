add_executable(part1)
target_include_directories(part1 PRIVATE include)
target_sources(part1 PRIVATE src/main.cpp)

# Find and link libcurl
find_package(CURL REQUIRED)
target_include_directories(part1 PRIVATE ${CURL_INCLUDE_DIRS})
target_link_libraries(part1 PRIVATE ${CURL_LIBRARIES})