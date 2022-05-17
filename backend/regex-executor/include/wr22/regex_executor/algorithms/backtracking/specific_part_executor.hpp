#pragma once

// wr22
#include <wr22/regex_executor/algorithms/backtracking/decision.hpp>
#include <wr22/regex_executor/algorithms/backtracking/interpreter.hpp>
#include <wr22/regex_executor/algorithms/backtracking/part_executor.hpp>
#include <wr22/regex_executor/utils/spanned_ref.hpp>
#include <wr22/regex_parser/regex/part.hpp>

#include <iostream>

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
        utils::SpannedRef<regex_parser::regex::part::Alternatives> part_ref);
    explicit SpecificPartExecutor(
        utils::SpannedRef<regex_parser::regex::part::Alternatives> part_ref,
        AlternativesDecision decision);
    bool execute(Interpreter& interpreter) const;

private:
    utils::SpannedRef<regex_parser::regex::part::Alternatives> m_part_ref;
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
    explicit QuantifierExecutor(utils::SpannedRef<Quantifier> part_ref);
    explicit QuantifierExecutor(
        utils::SpannedRef<Quantifier> part_ref,
        QuantifierDecision<Quantifier> decision,
        size_t already_matched);
    bool execute(Interpreter& interpreter);

private:
    size_t min_repetitions() const;
    std::optional<size_t> max_repetitions() const;
    bool num_repetitions_ok(size_t num_repetitions) const;
    static void greedy_walk_run_func(const instruction::Run::Context& ctx, Interpreter& interpreter);

    utils::SpannedRef<Quantifier> m_part_ref;
    QuantifierDecision<Quantifier> m_decision;
    size_t m_already_matched;
};

template <typename Quantifier>
using QuantifierExecutorBase = QuantifierExecutor<SpecificPartExecutor<Quantifier>, Quantifier>;

template <>
class SpecificPartExecutor<regex_parser::regex::part::Star> :
    public QuantifierExecutorBase<regex_parser::regex::part::Star> {
public:
    using QuantifierExecutorBase<regex_parser::regex::part::Star>::QuantifierExecutor;

    QuantifierType impl_quantifier_type() const;
    size_t impl_min_repetitions() const;
    std::optional<size_t> impl_max_repetitions() const;
};

template <>
class SpecificPartExecutor<regex_parser::regex::part::Plus> :
    public QuantifierExecutorBase<regex_parser::regex::part::Plus> {
public:
    using QuantifierExecutorBase<regex_parser::regex::part::Plus>::QuantifierExecutor;

    QuantifierType impl_quantifier_type() const;
    size_t impl_min_repetitions() const;
    std::optional<size_t> impl_max_repetitions() const;
};

template <>
class SpecificPartExecutor<regex_parser::regex::part::Optional> :
    public QuantifierExecutorBase<regex_parser::regex::part::Optional> {
public:
    using QuantifierExecutorBase<regex_parser::regex::part::Optional>::QuantifierExecutor;

    QuantifierType impl_quantifier_type() const;
    size_t impl_min_repetitions() const;
    std::optional<size_t> impl_max_repetitions() const;
};

}  // namespace wr22::regex_executor::algorithms::backtracking
