cmake_minimum_required(VERSION 3.12)
project(wr22-utils CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SRC_FILES "src/*.cpp")
add_library(wr22-utils ${SRC_FILES})
target_include_directories(wr22-utils PUBLIC "include")
target_compile_options(
    wr22-utils
    PUBLIC
        -Werror=return-type
        -Werror=switch
        -pedantic
)
