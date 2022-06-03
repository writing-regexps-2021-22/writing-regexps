#pragma once

// wr22
#include <wr22/regex_executor/algorithms/backtracking/step.hpp>
#include <wr22/regex_executor/capture.hpp>

// stl
#include <vector>
#include <optional>

// nlohmann
#include <nlohmann/json_fwd.hpp>

namespace wr22::regex_executor::algorithms::backtracking {

struct MatchResult {
    bool matched;
    std::optional<Captures> captures;
    std::vector<Step> steps;
};

void to_json(nlohmann::json& j, const MatchResult& result);

}  // namespace wr22::regex_executor::algorithms::backtracking
