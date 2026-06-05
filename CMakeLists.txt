cmake_minimum_required(VERSION 3.15)

project(NetworkScanner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES
    Src/*.cpp
    Src/*.h
)

add_executable(NetworkScanner ${SOURCES})

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(NetworkScanner PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(FetchContent)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
)

FetchContent_MakeAvailable(json)

target_link_libraries(
    NetworkScanner
    PRIVATE
        ws2_32
        iphlpapi
        nlohmann_json::nlohmann_json
)