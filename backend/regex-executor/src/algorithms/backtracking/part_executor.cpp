// wr22
#include <wr22/regex_executor/algorithms/backtracking/decision.hpp>
#include <wr22/regex_executor/algorithms/backtracking/interpreter.hpp>
#include <wr22/regex_executor/algorithms/backtracking/part_executor.hpp>
#include <wr22/regex_executor/utils/spanned_ref.hpp>
#include <wr22/regex_parser/regex/part.hpp>

#include <iostream>

namespace wr22::regex_executor::algorithms::backtracking {

namespace part = wr22::regex_parser::regex::part;
using utils::SpannedRef;
using wr22::regex_parser::regex::Part;
using wr22::regex_parser::span::Span;

namespace {
    using regex_parser::regex::CharacterClassData;
    bool char_class_matches(const CharacterClassData& data, char32_t c) {
        bool range_matched = false;
        for (const auto& range : data.ranges) {
            if (range.range.contains(c)) {
                range_matched = true;
                break;
            }
        }
        return range_matched ^ data.inverted;
    }
}  // namespace

template <typename T>
class SpecificPartExecutor {};

template <>
class SpecificPartExecutor<part::Empty> {
public:
    explicit SpecificPartExecutor([[maybe_unused]] SpannedRef<part::Empty> part_ref) {}

    bool execute([[maybe_unused]] Interpreter& interpreter) const {
        return true;
    }
};

template <>
class SpecificPartExecutor<part::Literal> {
public:
    explicit SpecificPartExecutor(SpannedRef<part::Literal> part_ref) : m_part_ref(part_ref) {}

    bool execute(Interpreter& interpreter) const {
        auto maybe_char = interpreter.current_char();
        if (!maybe_char.has_value()) {
            // Failure due to end of input.
            interpreter.add_step(step::MatchLiteral{
                .regex_span = m_part_ref.span(),
                .literal = m_part_ref.item().character,
                .result =
                    step::MatchLiteral::Failure{
                        .string_pos = interpreter.cursor(),
                        .failure_reason = failure_reasons::EndOfInput{},
                    },
            });
            return false;
        }
        auto c = maybe_char.value();
        if (c != m_part_ref.item().character) {
            // Failure due to a wrong character.
            interpreter.add_step(step::MatchLiteral{
                .regex_span = m_part_ref.span(),
                .literal = m_part_ref.item().character,
                .result =
                    step::MatchLiteral::Failure{
                        .string_pos = interpreter.cursor(),
                        .failure_reason = failure_reasons::OtherChar{},
                    },
            });
            return false;
        }

        // Success.
        interpreter.add_step(step::MatchLiteral{
            .regex_span = m_part_ref.span(),
            .literal = m_part_ref.item().character,
            .result =
                step::MatchLiteral::Success{
                    .string_span = Span::make_single_position(interpreter.cursor()),
                },
        });
        interpreter.advance();
        return true;
    }

private:
    SpannedRef<part::Literal> m_part_ref;
};

template <>
class SpecificPartExecutor<part::Wildcard> {
public:
    explicit SpecificPartExecutor(SpannedRef<part::Wildcard> part_ref) : m_part_ref(part_ref) {}

    bool execute(Interpreter& interpreter) const {
        auto maybe_char = interpreter.current_char();
        if (!maybe_char.has_value()) {
            // Failure due to end of input.
            interpreter.add_step(step::MatchWildcard{
                .regex_span = m_part_ref.span(),
                .result =
                    step::MatchWildcard::Failure{
                        .string_pos = interpreter.cursor(),
                        .failure_reason = failure_reasons::EndOfInput{},
                    },
            });
            return false;
        }

        interpreter.add_step(step::MatchWildcard{
            .regex_span = m_part_ref.span(),
            .result =
                step::MatchWildcard::Success{
                    .string_span = Span::make_single_position(interpreter.cursor()),
                },
        });
        interpreter.advance();
        return true;
    }

private:
    SpannedRef<part::Wildcard> m_part_ref;
};

template <>
class SpecificPartExecutor<part::Sequence> {
public:
    explicit SpecificPartExecutor(SpannedRef<part::Sequence> part_ref) : m_part_ref(part_ref) {}

    bool execute(Interpreter& interpreter) const {
        // We could have used `std::views::reverse` from C++20, but
        // clang doesn't seem to support it properly (at least the libstdc++ implementation).
        const auto& items = m_part_ref.item().items;
        for (auto it = items.rbegin(); it != items.rend(); ++it) {
            const auto& spanned_part = *it;
            auto ref = SpannedRef(spanned_part.part(), spanned_part.span());
            interpreter.add_instruction(instruction::Execute{.part = ref});
        }
        return true;
    }

private:
    SpannedRef<part::Sequence> m_part_ref;
};

template <>
class SpecificPartExecutor<part::Group> {
public:
    explicit SpecificPartExecutor(SpannedRef<part::Group> part_ref) : m_part_ref(part_ref) {}

    bool execute(Interpreter& interpreter) const {
        interpreter.add_instruction(instruction::Run{
            .ctx = std::monostate{},
            .fn =
                []([[maybe_unused]] const instruction::Run::Context& ctx,
                   Interpreter& interpreter) {
                    interpreter.add_step(step::EndGroup{
                        .string_pos = interpreter.cursor(),
                    });
                },
        });
        const auto& inner = *m_part_ref.item().inner;
        auto inner_ref = SpannedRef(inner.part(), inner.span());
        interpreter.add_instruction(instruction::Execute{inner_ref});
        interpreter.add_instruction(instruction::AddStep{step::BeginGroup{
            .regex_span = m_part_ref.span(),
            .string_pos = interpreter.cursor(),
        }});
        return true;
    }

private:
    SpannedRef<part::Group> m_part_ref;
};

template <>
class SpecificPartExecutor<part::Alternatives> {
public:
    explicit SpecificPartExecutor(SpannedRef<part::Alternatives> part_ref)
        : m_part_ref(part_ref), m_decision{.part_ref = part_ref.item()} {}

    explicit SpecificPartExecutor(
        SpannedRef<part::Alternatives> part_ref,
        AlternativesDecision decision)
        : m_part_ref(part_ref), m_decision(std::move(decision)) {}

    bool execute(Interpreter& interpreter) const {
        interpreter.add_decision(m_decision);
        const auto& alternative = m_part_ref.item().alternatives.at(m_decision.decision_index);
        auto ref = SpannedRef(alternative.part(), alternative.span());
        interpreter.add_instruction(instruction::Execute{ref});
        return true;
    }

private:
    SpannedRef<part::Alternatives> m_part_ref;
    AlternativesDecision m_decision;
};

template <>
class SpecificPartExecutor<part::CharacterClass> {
public:
    explicit SpecificPartExecutor(SpannedRef<part::CharacterClass> part_ref)
        : m_part_ref(part_ref) {}

    bool execute(Interpreter& interpreter) const {
        auto maybe_char = interpreter.current_char();
        if (!maybe_char.has_value()) {
            interpreter.add_step(step::MatchCharClass{
                .regex_span = m_part_ref.span(),
                .result =
                    step::MatchCharClass::Failure{
                        .string_pos = interpreter.cursor(),
                        .failure_reason = failure_reasons::EndOfInput{},
                    },
            });
            return false;
        }
        auto c = maybe_char.value();
        const auto& char_class_data = m_part_ref.item().data;
        if (!char_class_matches(char_class_data, c)) {
            interpreter.add_step(step::MatchCharClass{
                .regex_span = m_part_ref.span(),
                .result =
                    step::MatchCharClass::Failure{
                        .string_pos = interpreter.cursor(),
                        .failure_reason = failure_reasons::ExcludedChar{},
                    },
            });
            return false;
        }
        interpreter.add_step(step::MatchCharClass{
            .regex_span = m_part_ref.span(),
            .result =
                step::MatchCharClass::Success{
                    .string_span = Span::make_single_position(interpreter.cursor()),
                },
        });
        interpreter.advance();
        return true;
    }

private:
    SpannedRef<part::CharacterClass> m_part_ref;
};

template <typename Derived, typename Quantifier>
class QuantifierExecutor {
public:
    explicit QuantifierExecutor(SpannedRef<Quantifier> part_ref)
        : m_part_ref(part_ref), m_decision{}, m_already_matched(0) {}

    explicit QuantifierExecutor(
        SpannedRef<Quantifier> part_ref,
        QuantifierDecision<Quantifier> decision,
        size_t already_matched)
        : m_part_ref(part_ref), m_decision(decision), m_already_matched(already_matched) {}

    bool execute(Interpreter& interpreter) {
        std::cout << "  ~ greedy_walk" << std::endl;
        if (m_decision.num_repetitions.has_value()) {
            // Case 1: known number of repetitions.
            auto num_repetitions = m_decision.num_repetitions.value();

            // This can never be the case.
            if (m_already_matched > num_repetitions) {
                throw std::logic_error(
                    "QuantifierExecutor was told that it has matched more repetitions than it "
                    "was decided to, which is impossible unless there is a bug in the code");
            }

            // This can never be the case in practice (may change if/when lazy quantifiers
            // are implemented).
            if (m_already_matched < num_repetitions) {
                throw std::logic_error(
                    "QuantifierExecutor was told that it has matched less repetitions than it "
                    "was decided to, with the latter number being known. This is technically "
                    "possible, but can never arise in practive while executing a regular "
                    "expression unless there is a bug in the code");
            }

            // Here, `m_already_matched == num_repetitions`, so we are done.
            return true;
        } else {
            // Case 2: unknown number of repetitions.

            // This can never be the case in practice, as `m_already_matched` is set to a
            // non-zero value only when constructing `QuantifierExecutor` from the reconsidered
            // decision, at which point the exact number of repetitions is known.
            if (m_already_matched > 0) {
                throw std::logic_error(
                    "QuantifierExecutor was told that is has already matched some non-zero "
                    "number of repetitions, but the exact number of repetitions needed was not "
                    "known. This is technically possible, but can never arise in practice "
                    "while executing a regular expression unless there is a bug in the code");
            }

            // (5) Uninstall the error hook.
            interpreter.add_instruction(instruction::Run{
                .ctx = std::monostate{},
                .fn =
                    []([[maybe_unused]] const instruction::Run::Context& ctx,
                       Interpreter& interpreter) {
                        std::cout << "  ~ greedy_walk: pop error hook" << std::endl;
                        interpreter.pop_error_hook();
                    },
            });

            // (4) Run the greedy walk process.
            interpreter.add_instruction(instruction::Run{
                .ctx = m_part_ref,
                .fn = greedy_walk_run_func,
            });

            // (3) Install the error hook.
            interpreter.add_instruction(instruction::Run{
                .ctx = std::monostate{},
                .fn =
                    []([[maybe_unused]] const instruction::Run::Context& ctx,
                       Interpreter& interpreter) {
                        std::cout << "  ~ greedy_walk: push error hook" << std::endl;
                        auto decision_ref = DecisionRef{.index = interpreter.counter_at_offset(0)};
                        interpreter.pop_counter();

                        interpreter.push_error_hook(ErrorHook{
                            .ctx = decision_ref,
                            .fn =
                                [](const ErrorHook::Context& ctx, Interpreter& interpreter) {
                                    std::cout << "  ~ greedy_walk: error hook" << std::endl;
                                    auto actual_num_repetitions = interpreter.counter_at_offset(0);
                                    auto decision_ref = std::get<DecisionRef>(ctx.as_variant());
                                    // Set the number of repetitions to the actual one in the
                                    // decision made earlier.
                                    std::get<QuantifierDecision<Quantifier>>(
                                        interpreter.decision_snapshot_at(decision_ref)
                                            .decision.as_variant())
                                        .actual_num_repetitions = actual_num_repetitions;
                                    // Pop the repetition counter.
                                    interpreter.pop_counter();
                                },
                        });
                    },
            });

            // (2) Count the number of repetitions.
            interpreter.push_counter(m_already_matched);

            // (1) Make a snapshot and pass its index to the error hook.
            auto decision_ref = interpreter.add_decision(m_decision);
            interpreter.push_counter(decision_ref.index);
        }

        return true;
    }

private:
    size_t min_repetitions() const {
        return static_cast<const Derived*>(this)->impl_min_repetitions();
    }

    std::optional<size_t> max_repetitions() const {
        return static_cast<const Derived*>(this)->impl_max_repetitions();
    }

    bool num_repetitions_ok(size_t num_repetitions) const {
        auto min = min_repetitions();
        auto max = max_repetitions();
        return min <= num_repetitions && (!max.has_value() || num_repetitions <= max.value());
    }

    static void greedy_walk_run_func(const instruction::Run::Context& ctx, Interpreter& interpreter) {
        std::cout << "  ~ greedy_walk_run_func" << std::endl;
        // (4) Run quasi-recursively.
        interpreter.add_instruction(instruction::Run{
            .ctx = ctx,
            .fn = greedy_walk_run_func,
        });
        // (3) Increment the repetition counter.
        interpreter.add_instruction(instruction::Run{
            .ctx = std::monostate{},
            .fn =
                []([[maybe_unused]] const instruction::Run::Context& ctx,
                   Interpreter& interpreter) {
                    std::cout << "  ~ greedy_walk_run_func: increment" << std::endl;
                    auto& current_num_repetitions = interpreter.counter_at_offset(0);
                    ++current_num_repetitions;
                },
        });
        // (2) Match one item.
        const auto& part = std::get<SpannedRef<Quantifier>>(ctx.as_variant());
        const auto& inner = *part.item().inner;
        auto ref = SpannedRef<Part>(inner.part(), inner.span());
        interpreter.add_instruction(instruction::Execute{ref});

        // (1) Make a mini-snapshot.
        interpreter.add_instruction(instruction::Run{
            .ctx = std::monostate{},
            .fn =
                []([[maybe_unused]] const instruction::Run::Context& ctx,
                   Interpreter& interpreter) {
                    std::cout << "  ~ greedy_walk_run_func: make a mini snapshot" << std::endl;
                    interpreter.push_mini_snapshot();
                },
        });
    }

    SpannedRef<Quantifier> m_part_ref;
    QuantifierDecision<Quantifier> m_decision;
    size_t m_already_matched;
};

template <typename Quantifier>
using QuantifierExecutorBase = QuantifierExecutor<SpecificPartExecutor<Quantifier>, Quantifier>;

template <>
class SpecificPartExecutor<part::Star> : public QuantifierExecutorBase<part::Star> {
public:
    using QuantifierExecutorBase<part::Star>::QuantifierExecutor;

    QuantifierType impl_quantifier_type() const {
        return QuantifierType::Star;
    }

    size_t impl_min_repetitions() const {
        return 0;
    }

    std::optional<size_t> impl_max_repetitions() const {
        return std::nullopt;
    }
};

template <>
class SpecificPartExecutor<part::Plus> : public QuantifierExecutorBase<part::Plus> {
public:
    using QuantifierExecutorBase<part::Plus>::QuantifierExecutor;

    QuantifierType impl_quantifier_type() const {
        return QuantifierType::Plus;
    }

    size_t impl_min_repetitions() const {
        return 1;
    }

    std::optional<size_t> impl_max_repetitions() const {
        return std::nullopt;
    }
};

template <>
class SpecificPartExecutor<part::Optional> : public QuantifierExecutorBase<part::Optional> {
public:
    using QuantifierExecutorBase<part::Optional>::QuantifierExecutor;

    QuantifierType impl_quantifier_type() const {
        return QuantifierType::Optional;
    }

    size_t impl_min_repetitions() const {
        return 0;
    }

    std::optional<size_t> impl_max_repetitions() const {
        return 1;
    }
};

PartExecutor::PartExecutor(utils::SpannedRef<regex_parser::regex::Part> part_ref)
    : m_part_ref(part_ref) {}

bool PartExecutor::execute(Interpreter& interpreter) const {
    return m_part_ref.item().visit([&](const auto& part) {
        auto ref = SpannedRef(part, m_part_ref.span());
        using PartT = std::remove_cvref_t<decltype(part)>;
        auto executor = SpecificPartExecutor<PartT>(ref);
        return executor.execute(interpreter);
    });
}

}  // namespace wr22::regex_executor::algorithms::backtracking
