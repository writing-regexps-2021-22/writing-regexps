#pragma once

// wr22
#include "wr22/regex_executor/quantifier_type.hpp"
#include <wr22/regex_executor/algorithms/backtracking/decision.hpp>
#include <wr22/regex_executor/algorithms/backtracking/interpreter.hpp>
#include <wr22/regex_executor/algorithms/backtracking/part_executor.hpp>
#include <wr22/regex_executor/utils/spanned_ref.hpp>
#include <wr22/regex_parser/regex/part.hpp>

namespace wr22::regex_executor::algorithms::backtracking {

template <typename T>
class SpecificPartExecutor {};

template <>
class SpecificPartExecutor<regex_parser::regex::part::Empty> {
public:
    explicit SpecificPartExecutor(
        [[maybe_unused]] utils::SpannedRef<regex_parser::regex::part::Empty> part_ref);
    bool execute([[maybe_unused]] Interpreter& interpreter) const;
};

template <>
class SpecificPartExecutor<regex_parser::regex::part::Literal> {
public:
    explicit SpecificPartExecutor(utils::SpannedRef<regex_parser::regex::part::Literal> part_ref);
    bool execute(Interpreter& interpreter) const;

private:
    utils::SpannedRef<regex_parser::regex::part::Literal> m_part_ref;
};

template <>
class SpecificPartExecutor<regex_parser::regex::part::Wildcard> {
public:
    explicit SpecificPartExecutor(utils::SpannedRef<regex_parser::regex::part::Wildcard> part_ref);
    bool execute(Interpreter& interpreter) const;

private:
    utils::SpannedRef<regex_parser::regex::part::Wildcard> m_part_ref;
};

template <>
class SpecificPartExecutor<regex_parser::regex::part::Sequence> {
public:
    explicit SpecificPartExecutor(utils::SpannedRef<regex_parser::regex::part::Sequence> part_ref);
    bool execute(Interpreter& interpreter) const;

private:
    utils::SpannedRef<regex_parser::regex::part::Sequence> m_part_ref;
};

template <>
class SpecificPartExecutor<regex_parser::regex::part::Group> {
public:
    explicit SpecificPartExecutor(utils::SpannedRef<regex_parser::regex::part::Group> part_ref);
    bool execute(Interpreter& interpreter) const;

private:
    utils::SpannedRef<regex_parser::regex::part::Group> m_part_ref;
};

template <>
class SpecificPartExecutor<regex_parser::regex::part::Alternatives> {
public:
    explicit SpecificPartExecutor(
        utils::SpannedRef<regex_parser::regex::part::Alternatives> part_ref,
        utils::SpannedRef<regex_parser::regex::Part> part_var_ref);
    explicit SpecificPartExecutor(
        utils::SpannedRef<regex_parser::regex::part::Alternatives> part_ref,
        utils::SpannedRef<regex_parser::regex::Part> part_var_ref,
        AlternativesDecision decision);
    bool execute(Interpreter& interpreter);

private:
    utils::SpannedRef<regex_parser::regex::part::Alternatives> m_part_ref;
    utils::SpannedRef<regex_parser::regex::Part> m_part_var_ref;
    AlternativesDecision m_decision;
};

template <>
class SpecificPartExecutor<regex_parser::regex::part::CharacterClass> {
public:
    explicit SpecificPartExecutor(
        utils::SpannedRef<regex_parser::regex::part::CharacterClass> part_ref);
    bool execute(Interpreter& interpreter) const;

private:
    utils::SpannedRef<regex_parser::regex::part::CharacterClass> m_part_ref;
};

template <typename Derived, typename Quantifier>
class QuantifierExecutor {
public:
    struct StopImmediatelyTag {};

    explicit QuantifierExecutor(
        utils::SpannedRef<Quantifier> part_ref,
        utils::SpannedRef<regex_parser::regex::Part> part_var_ref);
    explicit QuantifierExecutor(
        utils::SpannedRef<Quantifier> part_ref,
        utils::SpannedRef<regex_parser::regex::Part> part_var_ref,
        StopImmediatelyTag tag);
    bool execute(Interpreter& interpreter);

    static size_t min_repetitions();
    static std::optional<size_t> max_repetitions();
    static QuantifierType quantifier_type();

private:
    static bool num_repetitions_ok(size_t num_repetitions);

    static bool greedy_walk_run_func(const instruction::Run::Context& ctx, Interpreter& interpreter);
    static bool finalize_run_func(const instruction::Run::Context& ctx, Interpreter& interpreter);

    utils::SpannedRef<Quantifier> m_part_ref;
    utils::SpannedRef<regex_parser::regex::Part> m_part_var_ref;
    bool m_stop_immediately;
};

template <typename Quantifier>
using QuantifierExecutorBase = QuantifierExecutor<SpecificPartExecutor<Quantifier>, Quantifier>;

template <>
class SpecificPartExecutor<regex_parser::regex::part::Star> :
    public QuantifierExecutorBase<regex_parser::regex::part::Star> {
public:
    using QuantifierExecutorBase<regex_parser::regex::part::Star>::QuantifierExecutor;

    static QuantifierType impl_quantifier_type();
    static size_t impl_min_repetitions();
    static std::optional<size_t> impl_max_repetitions();
};

template <>
class SpecificPartExecutor<regex_parser::regex::part::Plus> :
    public QuantifierExecutorBase<regex_parser::regex::part::Plus> {
public:
    using QuantifierExecutorBase<regex_parser::regex::part::Plus>::QuantifierExecutor;

    static QuantifierType impl_quantifier_type();
    static size_t impl_min_repetitions();
    static std::optional<size_t> impl_max_repetitions();
};

template <>
class SpecificPartExecutor<regex_parser::regex::part::Optional> :
    public QuantifierExecutorBase<regex_parser::regex::part::Optional> {
public:
    using QuantifierExecutorBase<regex_parser::regex::part::Optional>::QuantifierExecutor;

    static QuantifierType impl_quantifier_type();
    static size_t impl_min_repetitions();
    static std::optional<size_t> impl_max_repetitions();
};

}  // namespace wr22::regex_executor::algorithms::backtracking
