cmake_minimum_required(VERSION 3.12)
project(writing-regexps CXX)

find_package(fmt REQUIRED)
find_package(Boost 1.77 REQUIRED)
find_package(spdlog REQUIRED)
find_package(Crow)

add_subdirectory(unicode)
add_subdirectory(regex-parser)
add_subdirectory(regex-explainer)
add_subdirectory(regex-server)
