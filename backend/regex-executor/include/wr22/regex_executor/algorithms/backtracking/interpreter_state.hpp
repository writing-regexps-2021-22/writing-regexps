#pragma once

// wr22
#include <wr22/regex_executor/algorithms/backtracking/error_hook.hpp>
#include <wr22/regex_executor/algorithms/backtracking/instruction.hpp>

// stl
#include <stack>
#include <vector>

namespace wr22::regex_executor::algorithms::backtracking {

struct InterpreterState {
    // TODO: captures, if they belong here.
    size_t cursor = 0;
    std::vector<Instruction> instructions;
    std::stack<ErrorHook> error_hooks;
    std::vector<size_t> counters;
};

struct InterpreterStateSnapshot {
    InterpreterState state;
    Instruction current_instruction;
    size_t before_step;
};

struct InterpreterStateMiniSnapshot {
    size_t cursor;
    size_t before_step;
};

}  // namespace wr22::regex_executor::algorithms::backtracking
