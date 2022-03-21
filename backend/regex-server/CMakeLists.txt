cmake_minimum_required(VERSION 3.12)
project(wr22-regex-server CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

#option(REGEX_SERVER_BUILD_TESTS ON "Build tests for regex-server")

file(GLOB_RECURSE SRC_FILES "src/regex_server/*.cpp")
add_library(wr22-regex-server-library STATIC ${SRC_FILES})
target_include_directories(wr22-regex-server-library PUBLIC "include")
target_compile_options(
    wr22-regex-server-library
    PUBLIC
        -Werror=return-type
        -Werror=switch
        -pedantic
)
target_compile_definitions(
    wr22-regex-server-library
    PUBLIC
        SPDLOG_FMT_EXTERNAL
)
target_link_libraries(
    wr22-regex-server-library
    PUBLIC
        fmt
        wr22-regex-parser
)

add_executable(wr22-regex-server "src/main.cpp")
target_link_libraries(
    wr22-regex-server
    PRIVATE
        wr22-regex-server-library
)

#if (${REGEX_SERVER_BUILD_TESTS})
#    find_package(Catch2 2 REQUIRED)
#
#    file(GLOB_RECURSE TEST_FILES "test/src/*.cpp")
#    add_executable(wr22-regex-server-tests ${TEST_FILES})
#    target_include_directories(
#        wr22-regex-server-tests
#        PRIVATE
#            "include"
#            "test/include"
#    )
#    target_link_libraries(
#        wr22-regex-server-tests
#        PRIVATE
#            wr22-regex-server-library
#            Catch2::Catch2
#    )
#
#    add_test(
#        NAME wr22-regex-server-tests-target
#        COMMAND wr22-regex-server-tests
#    )
#endif ()
