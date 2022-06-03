// wr22
#include <wr22/regex_executor/algorithms/backtracking/decision.hpp>
#include <wr22/regex_executor/algorithms/backtracking/interpreter.hpp>
#include <wr22/regex_parser/regex/part.hpp>

#include <iostream>

namespace wr22::regex_executor::algorithms::backtracking {

namespace part = regex_parser::regex::part;

std::optional<AlternativesDecision> AlternativesDecision::reconsider(
    Interpreter& interpreter,
    InterpreterStateSnapshot snapshot) const {
    if (decision_index >= part_ref.get().alternatives.size()) {
        return std::nullopt;
    }

    auto new_decision = AlternativesDecision{
        .part_ref = part_ref,
        .initial_string_pos = initial_string_pos,
        .decision_index = decision_index + 1,
    };
    interpreter.restore_from_snapshot(std::move(snapshot));
    return new_decision;
}

template <typename PartT>
std::optional<QuantifierDecision<PartT>> QuantifierDecision<PartT>::reconsider(
    Interpreter& interpreter,
    InterpreterStateSnapshot snapshot) const {
    if (stop_here) {
        return std::nullopt;
    }
    auto new_decision = QuantifierDecision<PartT>{
        .stop_here = true,
    };
    interpreter.restore_from_snapshot(std::move(snapshot));
    return new_decision;
}

// Explicit instantiations.
template std::optional<QuantifierDecision<part::Star>> QuantifierDecision<part::Star>::reconsider(
    Interpreter&,
    InterpreterStateSnapshot) const;
template std::optional<QuantifierDecision<part::Plus>> QuantifierDecision<part::Plus>::reconsider(
    Interpreter&,
    InterpreterStateSnapshot) const;
template std::optional<QuantifierDecision<part::Optional>> QuantifierDecision<
    part::Optional>::reconsider(Interpreter&, InterpreterStateSnapshot) const;

}  // namespace wr22::regex_executor::algorithms::backtracking
