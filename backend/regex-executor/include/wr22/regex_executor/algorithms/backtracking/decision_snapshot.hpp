#pragma once

// wr22
#include <wr22/regex_executor/algorithms/backtracking/decision.hpp>
#include <wr22/regex_executor/algorithms/backtracking/interpreter_state.hpp>

namespace wr22::regex_executor::algorithms::backtracking {

struct DecisionSnapshot {
    InterpreterStateSnapshot snapshot;
    Decision decision;
};

}  // namespace wr22::regex_executor::algorithms::backtracking
