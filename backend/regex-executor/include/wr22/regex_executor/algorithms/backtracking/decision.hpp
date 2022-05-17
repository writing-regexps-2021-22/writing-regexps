#pragma once

// wr22
//#include <wr22/regex_executor/algorithms/backtracking/interpreter_state.hpp>
#include <wr22/regex_parser/regex/part.hpp>

// stl
#include <functional>
#include <optional>

namespace wr22::regex_executor::algorithms::backtracking {

struct Interpreter;
struct InterpreterStateSnapshot;

struct AlternativesDecision {
    using PartType = regex_parser::regex::part::Alternatives;

    // SAFETY: comes from borrowed `regex`, safe to keep as long
    // as the `Executor` is alive, provided its safety assumptions
    // are met.
    std::reference_wrapper<const PartType> part_ref;
    size_t decision_index = 0;

    std::optional<AlternativesDecision> reconsider(
        Interpreter& interpreter,
        InterpreterStateSnapshot snapshot) const;

    bool execute()
};

template <typename PartT>
struct QuantifierDecision {
    using PartType = PartT;
    std::optional<size_t> num_repetitions;
    std::optional<size_t> actual_num_repetitions;

    std::optional<QuantifierDecision<PartT>> reconsider(
        Interpreter& interpreter,
        InterpreterStateSnapshot snapshot) const;
};

using Decision = wr22::utils::Adt<
    AlternativesDecision,
    QuantifierDecision<regex_parser::regex::part::Star>,
    QuantifierDecision<regex_parser::regex::part::Plus>,
    QuantifierDecision<regex_parser::regex::part::Optional>>;

}  // namespace wr22::regex_executor::algorithms::backtracking
