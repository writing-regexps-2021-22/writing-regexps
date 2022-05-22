#pragma once

// wr22
#include <wr22/regex_executor/algorithms/backtracking/step.hpp>

// stl
#include <vector>

// nlohmann
#include <nlohmann/json_fwd.hpp>

namespace wr22::regex_executor::algorithms::backtracking {

struct MatchResult {
    bool matched;
    // TODO: captures.
    std::vector<Step> steps;
};

void to_json(nlohmann::json& j, const MatchResult& result);

}  // namespace wr22::regex_executor::algorithms::backtracking
