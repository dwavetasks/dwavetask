cmake_minimum_required(VERSION 3.10)
project(dwavetasks)

# Set C++14 standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable O3 optimizations by default
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

# Generate compile_commands.json for clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable clang-tidy (optional, only if installed)
find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
if(CLANG_TIDY_EXE)
    set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_EXE})
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
else()
    message(STATUS "clang-tidy not found, skipping static analysis")
endif()

add_subdirectory(part1)
add_subdirectory(part2)

# CURL Integration
find_package(CURL REQUIRED)
if(CURL_FOUND)
    message(STATUS "libcurl found: ${CURL_LIBRARIES}")
else()
    message(FATAL_ERROR "libcurl not found! Install libcurl-dev.")
endif()
