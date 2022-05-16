// wr22
#include <wr22/regex_executor/algorithms/backtracking/decision.hpp>

namespace wr22::regex_executor::algorithms::backtracking {

std::optional<AlternativesDecision> AlternativesDecision::reconsider() const {
    if (decision_index >= part_ref.get().alternatives.size()) {
        return std::nullopt;
    }
    return AlternativesDecision{
        .part_ref = part_ref,
        .decision_index = decision_index + 1,
    };
}

}  // namespace wr22::regex_executor::algorithms::backtracking
