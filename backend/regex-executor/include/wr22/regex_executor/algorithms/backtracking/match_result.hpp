#pragma once

// wr22
#include <wr22/regex_executor/algorithms/backtracking/step.hpp>

namespace wr22::regex_executor::algorithms::backtracking {

struct MatchResult {
    // TODO: captures.
    std::vector<Step> steps;
};
// TODO: json conversion.

}  // namespace wr22::regex_executor::algorithms::backtracking
