cmake_minimum_required(VERSION 3.12)
project(writing-regexps CXX)

option(WR22_STATIC_LINKING OFF "Link the executable statically")

set(FMT_TARGET fmt::fmt)
set(SPDLOG_TARGET spdlog::spdlog)
if (${WR22_STATIC_LINKING})
    set(Boost_USE_STATIC_LIBS ON)
    # Commented out because `spdlog::spdlog_header_only` forces linking to `fmt::fmt`, which
    # does not allow to get rid of the dependency on `libfmt.so.8` completely.
    #set(FMT_TARGET fmt::fmt-header-only)
    set(SPDLOG_TARGET spdlog::spdlog_header_only)
endif ()

find_package(fmt REQUIRED)
find_package(Boost 1.77 REQUIRED)
find_package(spdlog REQUIRED)
find_package(Crow REQUIRED)

add_subdirectory(utils)
add_subdirectory(unicode)
add_subdirectory(regex-parser)
add_subdirectory(regex-executor)
add_subdirectory(regex-server)
