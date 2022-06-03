#pragma once

// wr22
#include <wr22/regex_executor/algorithms/backtracking/error_hook.hpp>
#include <wr22/regex_executor/algorithms/backtracking/instruction.hpp>
#include <wr22/regex_executor/capture.hpp>

// stl
#include <stack>
#include <vector>

namespace wr22::regex_executor::algorithms::backtracking {

struct InterpreterState {
    size_t cursor = 0;
    std::vector<Instruction> instructions;
    std::stack<ErrorHook> error_hooks;
    std::vector<size_t> counters;
    Captures captures;
    size_t capture_counter = 1;
};

struct InterpreterStateSnapshot {
    InterpreterState state;
    utils::SpannedRef<regex_parser::regex::Part> decision_making_part;
    size_t before_step;
};

struct InterpreterStateMiniSnapshot {
    size_t cursor;
    size_t before_step;
};

}  // namespace wr22::regex_executor::algorithms::backtracking
