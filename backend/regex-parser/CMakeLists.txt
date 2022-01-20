cmake_minimum_required(VERSION 3.12)
project(wr22-regex-parser CXX)
set(CMAKE_CXX_STANDARD 20)

option(REGEX_PARSER_BUILD_TESTS ON "Build tests for regex-parser")

file(GLOB_RECURSE SRC_FILES "src/*.cpp")
add_library(wr22-regex-parser ${SRC_FILES})
target_include_directories(wr22-regex-parser PUBLIC "include")
target_compile_options(
    wr22-regex-parser
    PUBLIC
        -Werror=return-type
)

if (${REGEX_PARSER_BUILD_TESTS})
    find_package(Catch2 2 REQUIRED)

    file(GLOB_RECURSE TEST_FILES "test/src/*.cpp")
    add_executable(wr22-regex-parser-tests ${TEST_FILES})
    target_include_directories(
        wr22-regex-parser-tests
        PRIVATE
            "include"
            "test/include"
    )
    target_link_libraries(
        wr22-regex-parser-tests
        PRIVATE
            wr22-regex-parser
            Catch2::Catch2
    )

    add_test(
        NAME wr22-regex-parser-tests-target
        COMMAND wr22-regex-parser-tests
    )
endif ()
