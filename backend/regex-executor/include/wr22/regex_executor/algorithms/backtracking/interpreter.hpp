#pragma once

// wr22
#include <wr22/regex_executor/algorithms/backtracking/decision.hpp>
#include <wr22/regex_executor/algorithms/backtracking/decision_ref.hpp>
#include <wr22/regex_executor/algorithms/backtracking/error_hook.hpp>
#include <wr22/regex_executor/algorithms/backtracking/instruction.hpp>
#include <wr22/regex_executor/algorithms/backtracking/step.hpp>
#include <wr22/regex_parser/regex/part.hpp>

// stl
#include <stack>
#include <string_view>
#include <vector>

namespace wr22::regex_executor::algorithms::backtracking {

struct InterpreterState {
    // TODO: captures, if they belong here.
    size_t cursor = 0;
    std::stack<Instruction> instructions;
    std::stack<ErrorHook> error_hooks;
    std::vector<size_t> counters;
};

struct InterpreterStateSnapshot {
    InterpreterState state;
    size_t before_decision;
    size_t before_step;
};

class Interpreter {
public:
    explicit Interpreter(const std::u32string_view& string_ref);

    std::optional<char32_t> current_char() const;
    void advance();
    void make_snapshot();

    DecisionRef add_decision(Decision decision);
    const Decision& decision_at(DecisionRef ref) const;
    Decision& decision_at(DecisionRef ref);

    void add_instruction(Instruction instruction);
    void add_step(Step step);

    void push_error_hook(ErrorHook hook);
    void pop_error_hook();

    void push_counter(size_t initial);
    void pop_counter();
    size_t& counter_at_offset(size_t offset);
    size_t counter_at_offset(size_t offset) const;

    size_t cursor() const;
    bool finished() const;

    void run_instruction();

private:
    size_t parse_counter_offset(size_t offset) const;

    std::u32string_view m_string_ref;
    InterpreterState m_current_state;
    // Not using std::stack to be able to truncate to a specific size.
    std::vector<Decision> m_decision_stack;
    std::stack<InterpreterStateSnapshot> m_snapshots;
    std::vector<Step> m_steps;
};

}  // namespace wr22::regex_executor::algorithms::backtracking
