cmake_minimum_required(VERSION 3.12)
project(regex-parser CXX)
set(CMAKE_CXX_STANDARD 20)

option(REGEX_PARSER_BUILD_TESTS ON "Build tests for regex-parser")

include(CMakeRC)
cmrc_add_resource_library(
    regex-parser-grammar
    NAMESPACE grammar
    src/parser/regex.peg
)

file(GLOB_RECURSE SRC_FILES "src/*.cpp")
add_library(regex-parser ${SRC_FILES})
target_include_directories(regex-parser PRIVATE "include")
target_link_libraries(regex-parser PRIVATE regex-parser-grammar)
target_compile_options(
    regex-parser
    PUBLIC
        -Werror=return-type
)

if (${REGEX_PARSER_BUILD_TESTS})
    find_package(Catch2 2 REQUIRED)

    file(GLOB_RECURSE TEST_FILES "test/src/*.cpp")
    add_executable(regex-parser-tests ${TEST_FILES})
    target_include_directories(
        regex-parser-tests
        PRIVATE
            "include"
            "test/include"
    )
    target_link_libraries(
        regex-parser-tests
        PRIVATE
            regex-parser
            Catch2::Catch2
    )

    add_test(
        NAME regex-parser-tests-target
        COMMAND regex-parser-tests
    )
endif ()
