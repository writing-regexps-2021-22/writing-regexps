// wr22
#include <wr22/regex_executor/algorithms/backtracking/match_failure.hpp>

namespace wr22::regex_executor::algorithms::backtracking {

const char* MatchFailure::what() const noexcept {
    return "String does not match the regular expression";
}

}  // namespace wr22::regex_executor::algorithms::backtracking
