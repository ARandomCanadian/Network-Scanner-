cmake_minimum_required(VERSION 3.15)

project(NetworkScanner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Collect all source files
file(GLOB_RECURSE SOURCES
    src/*.cpp
    src/*.h
)

add_executable(NetworkScanner ${SOURCES})

# Optional: warnings (good for security/cyber projects)
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(NetworkScanner PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_link_libraries(NetworkScanner ws2_32)