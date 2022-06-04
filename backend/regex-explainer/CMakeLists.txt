cmake_minimum_required(VERSION 3.12)
project(wr22-regex-explainer CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SRC_FILES "src/*.cpp")
add_library(wr22-regex-explainer ${SRC_FILES})
target_include_directories(wr22-regex-explainer PUBLIC "include")
target_compile_options(
        wr22-regex-explainer
        PUBLIC
        -Werror=return-type
        -Werror=switch
        -pedantic
)
target_link_libraries(
        wr22-regex-explainer
        PUBLIC
        "${FMT_TARGET}"
        wr22-regex-parser
        wr22-unicode
)
