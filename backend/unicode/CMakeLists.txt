cmake_minimum_required(VERSION 3.12)
project(wr22-unicode CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SRC_FILES "src/*.cpp")
add_library(wr22-unicode ${SRC_FILES})
target_include_directories(wr22-unicode PUBLIC "include")
target_compile_options(
    wr22-unicode
    PUBLIC
        -Werror=return-type
        -Werror=switch
        -pedantic
)
target_link_libraries(
    wr22-unicode
    PUBLIC
        Boost::boost
)
