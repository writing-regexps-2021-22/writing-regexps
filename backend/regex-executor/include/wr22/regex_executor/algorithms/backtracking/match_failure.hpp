#pragma once

// stl
#include <exception>

// fmt
#include <fmt/compile.h>
#include <fmt/core.h>

namespace wr22::regex_executor::algorithms::backtracking {

struct MatchFailure : public std::exception {
    const char* what() const noexcept override;
};

}  // namespace wr22::regex_executor::algorithms::backtracking
