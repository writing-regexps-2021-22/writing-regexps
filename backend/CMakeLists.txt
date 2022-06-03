cmake_minimum_required(VERSION 3.12)
project(writing-regexps CXX)

option(WR22_STATIC_LINKING OFF "Link the executable statically")
option(
    WR22_STATIC_LINKING_SPDLOG_HACK
    ON
    "When linking statically, patch the `spdlog::spdlog_header_only` target to remove dependency on `fmt::fmt`"
)

find_package(spdlog REQUIRED)

set(FMT_TARGET fmt::fmt)
set(SPDLOG_TARGET spdlog::spdlog)
if (${WR22_STATIC_LINKING})
    set(Boost_USE_STATIC_LIBS ON)
    set(SPDLOG_TARGET spdlog::spdlog_header_only)
    if (${WR22_STATIC_LINKING_SPDLOG_HACK})
        set(FMT_TARGET fmt::fmt-header-only)
        get_target_property(link_libraries ${SPDLOG_TARGET} INTERFACE_LINK_LIBRARIES)
        list(FIND link_libraries fmt::fmt index)
        if (${index} EQUAL -1)
            message(WARNING "${SPDLOG_TARGET} does not depend on fmt::fmt")
        else ()
            message(STATUS "Patching ${SPDLOG_TARGET} to depend on ${FMT_TARGET} instead of fmt::fmt")
            list(REMOVE_AT link_libraries ${index})
            list(INSERT link_libraries ${index} ${FMT_TARGET})
            set_target_properties(
                ${SPDLOG_TARGET}
                PROPERTIES
                    INTERFACE_LINK_LIBRARIES "${link_libraries}"
            )
        endif ()
    endif ()
endif ()

find_package(Boost 1.77 REQUIRED)
find_package(fmt REQUIRED)
find_package(Crow REQUIRED)

add_subdirectory(utils)
add_subdirectory(unicode)
add_subdirectory(regex-parser)
add_subdirectory(regex-explainer)
add_subdirectory(regex-executor)
add_subdirectory(regex-server)
