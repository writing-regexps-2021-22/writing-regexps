#pragma once

// wr22
#include <wr22/regex_executor/algorithms/backtracking/decision_ref.hpp>
#include <wr22/regex_executor/algorithms/backtracking/decision_snapshot.hpp>
#include <wr22/regex_executor/algorithms/backtracking/instruction.hpp>
#include <wr22/regex_executor/algorithms/backtracking/interpreter_state.hpp>
#include <wr22/regex_executor/algorithms/backtracking/step.hpp>
#include <wr22/regex_executor/regex.hpp>
#include <wr22/regex_parser/regex/part.hpp>

// stl
#include <functional>
#include <stack>
#include <string_view>
#include <vector>

namespace wr22::regex_executor::algorithms::backtracking {

class Interpreter {
public:
    Interpreter(const Regex& regex, const std::u32string_view& string_ref);

    std::optional<char32_t> current_char() const;
    void advance();

    DecisionRef add_decision(
        Decision decision,
        utils::SpannedRef<regex_parser::regex::Part> decision_making_part);
    const DecisionSnapshot& decision_snapshot_at(DecisionRef ref) const;
    DecisionSnapshot& decision_snapshot_at(DecisionRef ref);
    std::optional<DecisionRef> last_decision_ref() const;
    void restore_from_snapshot(InterpreterStateSnapshot snapshot);

    void push_mini_snapshot();
    void pop_mini_snapshot();
    std::optional<std::reference_wrapper<const InterpreterStateMiniSnapshot>> last_mini_snapshot()
        const;

    void add_instruction(Instruction instruction);
    void add_step(Step step);

    void push_error_hook(ErrorHook hook);
    void pop_error_hook();
    const ErrorHook& last_error_hook() const;
    ErrorHook& last_error_hook();

    void push_counter(size_t initial);
    void pop_counter();
    size_t& counter_at_offset(size_t offset);
    size_t counter_at_offset(size_t offset) const;

    size_t cursor() const;
    bool finished() const;

    void run_instruction();
    void finalize();
    void finalize_error();

    void add_indexed_capture(Capture capture);
    void add_named_capture(std::string_view name, Capture capture);

    std::vector<Step> into_steps() &&;

    const InterpreterState& current_state() const;
    InterpreterState& current_state();

private:
    size_t parse_counter_offset(size_t offset) const;

    std::u32string_view m_string_ref;
    InterpreterState m_current_state;
    std::vector<DecisionSnapshot> m_decision_snapshots;
    std::stack<InterpreterStateMiniSnapshot> m_mini_snapshots;
    std::vector<Step> m_steps;
};

}  // namespace wr22::regex_executor::algorithms::backtracking
