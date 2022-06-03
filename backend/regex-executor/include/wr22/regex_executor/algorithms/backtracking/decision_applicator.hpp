#pragma once

// wr22
#include <wr22/regex_executor/algorithms/backtracking/decision.hpp>
#include <wr22/regex_executor/algorithms/backtracking/failure_reason.hpp>
#include <wr22/regex_executor/algorithms/backtracking/interpreter.hpp>
#include <wr22/regex_executor/algorithms/backtracking/specific_part_executor.hpp>
#include <wr22/regex_executor/algorithms/backtracking/step.hpp>
#include <wr22/regex_executor/utils/spanned_ref.hpp>
#include <wr22/regex_parser/regex/part.hpp>

// stl
#include <algorithm>
#include <type_traits>

namespace wr22::regex_executor::algorithms::backtracking {

template <typename T>
struct DecisionApplicator {};

template <>
struct DecisionApplicator<AlternativesDecision> {
    using PartType = regex_parser::regex::part::Alternatives;
    using ExecutorType = SpecificPartExecutor<PartType>;

    static ExecutorType construct_executor(
        utils::SpannedRef<PartType> part_ref,
        utils::SpannedRef<regex_parser::regex::Part> part_var_ref,
        AlternativesDecision decision) {
        return ExecutorType(part_ref, part_var_ref, std::move(decision));
    }

    static void finalize_exhausted(
        [[maybe_unused]] AlternativesDecision decision,
        utils::SpannedRef<PartType> part_ref,
        Interpreter& interpreter) {
        interpreter.add_step(step::FinishAlternatives{
            .regex_span = part_ref.span(),
            .result =
                step::FinishAlternatives::Failure{
                    .string_pos = interpreter.cursor(),
                    .failure_reason = failure_reasons::OptionsExhausted{},
                },
        });
    }
};

template <typename Quantifier>
// Formatting disabled because clang-format fails terribly with requires-clauses.
// clang-format off
requires
    std::is_same_v<Quantifier, regex_parser::regex::part::Star>
    || std::is_same_v<Quantifier, regex_parser::regex::part::Plus>
    || std::is_same_v<Quantifier, regex_parser::regex::part::Optional>
// clang-format on
struct DecisionApplicator<QuantifierDecision<Quantifier>> {
    using PartType = Quantifier;
    using ExecutorType = SpecificPartExecutor<PartType>;

    static ExecutorType construct_executor(
        utils::SpannedRef<PartType> part_ref,
        utils::SpannedRef<regex_parser::regex::Part> part_var_ref,
        [[maybe_unused]] QuantifierDecision<PartType> decision) {
        return ExecutorType(part_ref, part_var_ref, typename ExecutorType::StopImmediatelyTag{});
    }

    static void finalize_exhausted(
        QuantifierDecision<Quantifier> decision,
        utils::SpannedRef<PartType> part_ref,
        Interpreter& interpreter) {
        if (decision.is_first) {
            interpreter.add_step(step::FinishQuantifier{
                .quantifier_type = ExecutorType::quantifier_type(),
                .regex_span = part_ref.span(),
                .result =
                    step::FinishQuantifier::Failure{
                        .string_pos = interpreter.cursor(),
                        .failure_reason = failure_reasons::OptionsExhausted{},
                    },
            });
        }
    }
};

}  // namespace wr22::regex_executor::algorithms::backtracking
