cmake_minimum_required(VERSION 3.12)
project(wr22-regex-server CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

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
        fmt::fmt
        Crow::Crow
        spdlog::spdlog
        wr22-regex-parser
        wr22-regex-explainer
)

add_executable(wr22-regex-server "src/main.cpp")
target_link_libraries(
    wr22-regex-server
    PRIVATE
        wr22-regex-server-library
)
