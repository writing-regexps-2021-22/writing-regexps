// wr22
#include "wr22/regex_parser/regex/part.hpp"
#include <wr22/regex_executor/algorithms/backtracking/decision.hpp>
#include <wr22/regex_executor/algorithms/backtracking/interpreter.hpp>

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
        .decision_index = decision_index + 1,
    };
    interpreter.restore_from_snapshot(std::move(snapshot));
    return std::move(new_decision);
}

template <typename PartT>
std::optional<QuantifierDecision<PartT>> QuantifierDecision<PartT>::reconsider(
    Interpreter& interpreter,
    InterpreterStateSnapshot snapshot) const {
    auto current_num_repetitions = num_repetitions.has_value() ? num_repetitions.value()
                                                               : actual_num_repetitions.value();
    // TODO: custom min.
    if (current_num_repetitions == 0) {
        return std::nullopt;
    }
    auto new_num_repetitions = current_num_repetitions - 1;
    auto new_decision = QuantifierDecision<PartT>{
        .num_repetitions = new_num_repetitions,
    };

    auto& state = interpreter.current_state();
    state.error_hooks = std::move(snapshot.state.error_hooks);
    state.instructions = std::move(snapshot.state.instructions);
    state.counters = std::move(snapshot.state.counters);
    //if (state.counters.empty()) {
    //    throw std::logic_error("")
    //}
    //state.counters.back() += new_num_repetitions;

    auto maybe_mini_snapshot = interpreter.last_mini_snapshot();
    if (!maybe_mini_snapshot.has_value()) {
        throw std::logic_error("No mini-snapshot exists, but QuantifierDecision<...> has not yet "
                               "exhausted its options");
    }
    auto mini_snapshot = maybe_mini_snapshot.value().get();
    state.cursor = mini_snapshot.cursor;
    auto before_step = mini_snapshot.before_step;
    interpreter.pop_mini_snapshot();
    interpreter.add_step(step::Backtrack{
        .string_pos = interpreter.cursor(),
        .continue_after_step = before_step - 1,
    });

    return std::move(new_decision);
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
