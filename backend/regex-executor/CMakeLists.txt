cmake_minimum_required(VERSION 3.12)
project(wr22-regex-executor CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

option(REGEX_EXECUTOR_BUILD_TESTS ON "Build tests for regex-executor")

file(GLOB_RECURSE SRC_FILES "src/*.cpp")
add_library(wr22-regex-executor ${SRC_FILES})
target_include_directories(wr22-regex-executor PUBLIC "include")
target_compile_options(
    wr22-regex-executor
    PUBLIC
        -Werror=return-type
        -Werror=switch
        -pedantic
)
target_link_libraries(
    wr22-regex-executor
    PUBLIC
        fmt
        wr22-unicode
        wr22-regex-parser
)

if (${REGEX_EXECUTOR_BUILD_TESTS})
    find_package(Catch2 2 REQUIRED)

    file(GLOB_RECURSE TEST_FILES "test/src/*.cpp")
    add_executable(wr22-regex-executor-tests ${TEST_FILES})
    target_include_directories(
        wr22-regex-executor-tests
        PRIVATE
            "include"
            "test/include"
    )
    target_link_libraries(
        wr22-regex-executor-tests
        PRIVATE
            wr22-regex-executor
            Catch2::Catch2
    )

    add_test(
        NAME wr22-regex-executor-tests-target
        COMMAND wr22-regex-executor-tests
    )
endif ()
