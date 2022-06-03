// wr22
#include <wr22/regex_executor/algorithms/backtracking/match_result.hpp>

// nlohmann
#include <nlohmann/json.hpp>

namespace wr22::regex_executor::algorithms::backtracking {

void to_json(nlohmann::json& j, const MatchResult& result) {
    j = nlohmann::json::object();
    j["matched"] = result.matched;
    if (result.captures.has_value()) {
        j["captures"] = result.captures.value();
    }
    j["steps"] = result.steps;
}

}  // namespace wr22::regex_executor::algorithms::backtracking
