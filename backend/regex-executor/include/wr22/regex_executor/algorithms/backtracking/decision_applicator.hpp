#pragma once

// wr22
#include <algorithm>
#include <wr22/regex_executor/algorithms/backtracking/decision.hpp>
#include <wr22/regex_executor/algorithms/backtracking/specific_part_executor.hpp>
#include <wr22/regex_executor/utils/spanned_ref.hpp>
#include <wr22/regex_parser/regex/part.hpp>

// stl
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
        AlternativesDecision decision) {
        return ExecutorType(part_ref, std::move(decision));
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
        QuantifierDecision<PartType> decision) {
        auto num_repetitions = decision.num_repetitions.value();
        return ExecutorType(part_ref, std::move(decision), num_repetitions);
    }
};

}  // namespace wr22::regex_executor::algorithms::backtracking
