// wr22
#include <wr22/regex_executor/algorithms/backtracking/executor.hpp>
#include <wr22/regex_executor/algorithms/backtracking/step.hpp>
#include <wr22/regex_parser/regex/character_class_data.hpp>
#include <wr22/regex_parser/regex/part.hpp>
#include <wr22/utils/adt.hpp>

// stl
#include <functional>
#include <optional>
#include <stack>
#include <string_view>
#include <type_traits>

namespace wr22::regex_executor::algorithms::backtracking {

namespace {
    namespace part = regex_parser::regex::part;
    using regex_parser::regex::CharacterClassData;
    using regex_parser::regex::Part;
    using regex_parser::regex::SpannedPart;
    using regex_parser::span::Span;

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

    template <typename T>
    class PartExecutor {};

    struct AlternativesDecision {
        using PartType = part::Alternatives;

        // SAFETY: comes from borrowed `regex`, safe to keep as long
        // as the `Executor` is alive, provided its safety assumptions
        // are met.
        std::reference_wrapper<const part::Alternatives> part_ref;
        size_t decision_index = 0;

        std::optional<AlternativesDecision> reconsider() const {
            if (decision_index >= part_ref.get().alternatives.size()) {
                return std::nullopt;
            }
            return AlternativesDecision{
                .part_ref = part_ref,
                .decision_index = decision_index + 1,
            };
        }
    };

    template <typename PartT>
    struct QuantifierDecision {
        using PartType = PartT;
        size_t repetition_num = 0;
        bool match_this = true;
    };

    struct MatchingState {
        // TODO: captures.
        size_t cursor = 0;
    };

    using Decision = utils::Adt<AlternativesDecision>;

    struct DecisionSnapshot {
        Decision decision;
        // TODO: use delta coding/state sharing if memory footprint becomes too high.
        // The current implementation copies the current `MatchingState` each time
        // it commits a decision. This is simple and might be fast when a lot of shallow backtracking
        // is needed, but is probably suboptimal in the average case. However, a more optimal
        // implementation would require time and effort, and the real benefits are questionable.
        MatchingState state_snapshot;
    };

    struct MatchingProcessState {
        explicit MatchingProcessState(std::u32string_view string_ref) : string_ref(string_ref) {}

        std::stack<DecisionSnapshot> decisions;
        MatchingState current_state;
        std::u32string_view string_ref;
        std::vector<Step> steps;

        std::optional<char32_t> current_char() const {
            if (current_state.cursor >= string_ref.size()) {
                return std::nullopt;
            }
            return string_ref[current_state.cursor];
        }

        void advance() {
            ++current_state.cursor;
        }

        DecisionSnapshot make_decision_snapshot(Decision decision) const {
            return DecisionSnapshot{
                .decision = std::move(decision),
                .state_snapshot = current_state,
            };
        }
    };

    template <typename T>
    class SpannedRef {
    public:
        SpannedRef(const T& item, Span span) : m_item(item), m_span(span) {}

        const T& item() const {
            return m_item.get();
        }

        Span span() const {
            return m_span;
        }

    private:
        std::reference_wrapper<const T> m_item;
        Span m_span;
    };

    template <>
    class PartExecutor<Part> {
    public:
        explicit PartExecutor(SpannedRef<Part> part_ref) : m_part_ref(part_ref) {}

        bool execute(MatchingProcessState& process) const;

    private:
        SpannedRef<Part> m_part_ref;
    };

    template <>
    class PartExecutor<part::Empty> {
    public:
        explicit PartExecutor([[maybe_unused]] SpannedRef<part::Empty> part_ref) {}

        bool execute([[maybe_unused]] MatchingProcessState& process) const {
            return true;
        }
    };

    template <>
    class PartExecutor<part::Literal> {
    public:
        explicit PartExecutor(SpannedRef<part::Literal> part_ref) : m_part_ref(part_ref) {}

        bool execute(MatchingProcessState& process) const {
            auto maybe_char = process.current_char();
            if (!maybe_char.has_value()) {
                // Failure due to end of input.
                process.steps.push_back(step::MatchLiteral{
                    .regex_span = m_part_ref.span(),
                    .literal = m_part_ref.item().character,
                    .result =
                        step::MatchLiteral::Failure{
                            .string_pos = process.current_state.cursor,
                            .failure_reason = failure_reasons::EndOfInput{},
                        },
                });
                return false;
            }
            auto c = maybe_char.value();
            if (c != m_part_ref.item().character) {
                // Failure due to a wrong character.
                process.steps.push_back(step::MatchLiteral{
                    .regex_span = m_part_ref.span(),
                    .literal = m_part_ref.item().character,
                    .result =
                        step::MatchLiteral::Failure{
                            .string_pos = process.current_state.cursor,
                            .failure_reason = failure_reasons::OtherChar{},
                        },
                });
                return false;
            }

            // Success.
            process.steps.push_back(step::MatchLiteral{
                .regex_span = m_part_ref.span(),
                .literal = m_part_ref.item().character,
                .result =
                    step::MatchLiteral::Success{
                        .string_span = Span::make_single_position(process.current_state.cursor),
                    },
            });
            process.advance();
            return true;
        }

    private:
        SpannedRef<part::Literal> m_part_ref;
    };

    template <>
    class PartExecutor<part::Wildcard> {
    public:
        explicit PartExecutor(SpannedRef<part::Wildcard> part_ref) : m_part_ref(part_ref) {}

        bool execute(MatchingProcessState& process) const {
            auto maybe_char = process.current_char();
            if (!maybe_char.has_value()) {
                // Failure due to end of input.
                process.steps.push_back(step::MatchWildcard{
                    .regex_span = m_part_ref.span(),
                    .result =
                        step::MatchWildcard::Failure{
                            .string_pos = process.current_state.cursor,
                            .failure_reason = failure_reasons::EndOfInput{},
                        },
                });
                return false;
            }

            process.steps.push_back(step::MatchWildcard{
                .regex_span = m_part_ref.span(),
                .result =
                    step::MatchWildcard::Success{
                        .string_span = Span::make_single_position(process.current_state.cursor),
                    },
            });
            process.advance();
            return true;
        }

    private:
        SpannedRef<part::Wildcard> m_part_ref;
    };

    template <>
    class PartExecutor<part::Sequence> {
    public:
        explicit PartExecutor(SpannedRef<part::Sequence> part_ref) : m_part_ref(part_ref) {}

        bool execute(MatchingProcessState& process) const {
            for (const auto& spanned_part : m_part_ref.item().items) {
                auto ref = SpannedRef(spanned_part.part(), spanned_part.span());
                auto item_executor = PartExecutor<Part>(ref);
                auto success = item_executor.execute(process);
                if (!success) {
                    return false;
                }
            }
            return true;
        }

    private:
        SpannedRef<part::Sequence> m_part_ref;
    };

    template <>
    class PartExecutor<part::Group> {
    public:
        explicit PartExecutor(SpannedRef<part::Group> part_ref) : m_part_ref(part_ref) {}

        bool execute(MatchingProcessState& process) const {
            process.steps.push_back(step::BeginGroup{
                .regex_span = m_part_ref.span(),
                .string_pos = process.current_state.cursor,
            });
            const auto& inner = *m_part_ref.item().inner;
            auto inner_ref = SpannedRef(inner.part(), inner.span());
            auto inner_executor = PartExecutor<Part>(inner_ref);
            auto success = inner_executor.execute(process);
            if (!success) {
                return false;
            }
            process.steps.push_back(step::EndGroup{
                .string_pos = process.current_state.cursor,
            });
            return true;
        }

    private:
        SpannedRef<part::Group> m_part_ref;
    };

    template <>
    class PartExecutor<part::Alternatives> {
    public:
        explicit PartExecutor(SpannedRef<part::Alternatives> part_ref)
            : m_part_ref(part_ref), m_decision{.part_ref = part_ref.item()} {}

        explicit PartExecutor(SpannedRef<part::Alternatives> part_ref, AlternativesDecision decision)
            : m_part_ref(part_ref), m_decision(std::move(decision)) {}

        bool execute(MatchingProcessState& process) const {
            auto decision_snapshot = process.make_decision_snapshot(m_decision);
            process.decisions.push(std::move(decision_snapshot));
            const auto& alternative = m_part_ref.item().alternatives.at(m_decision.decision_index);
            auto ref = SpannedRef(alternative.part(), alternative.span());
            auto executor = PartExecutor<Part>(ref);
            return executor.execute(process);
        }

    private:
        SpannedRef<part::Alternatives> m_part_ref;
        AlternativesDecision m_decision;
    };

    template <>
    class PartExecutor<part::CharacterClass> {
    public:
        explicit PartExecutor(SpannedRef<part::CharacterClass> part_ref) : m_part_ref(part_ref) {}

        bool execute(MatchingProcessState& process) const {
            auto maybe_char = process.current_char();
            if (!maybe_char.has_value()) {
                process.steps.push_back(step::MatchCharClass{
                    .regex_span = m_part_ref.span(),
                    .result =
                        step::MatchCharClass::Failure{
                            .string_pos = process.current_state.cursor,
                            .failure_reason = failure_reasons::EndOfInput{},
                        },
                });
                return false;
            }
            auto c = maybe_char.value();
            const auto& char_class_data = m_part_ref.item().data;
            if (!char_class_matches(char_class_data, c)) {
                process.steps.push_back(step::MatchCharClass{
                    .regex_span = m_part_ref.span(),
                    .result =
                        step::MatchCharClass::Failure{
                            .string_pos = process.current_state.cursor,
                            .failure_reason = failure_reasons::ExcludedChar{},
                        },
                });
                return false;
            }
            process.steps.push_back(step::MatchCharClass{
                .regex_span = m_part_ref.span(),
                .result =
                    step::MatchCharClass::Success{
                        .string_span = Span::make_single_position(process.current_state.cursor),
                    },
            });
            process.advance();
            return true;
        }

    private:
        SpannedRef<part::CharacterClass> m_part_ref;
    };

    template <typename Derived, typename Quantifier>
    class QuantifierExecutor {
        //static_assert(
        //    std::is_base_of_v<QuantifierExecutor<Derived, Quantifier>, Derived>,
        //    "QuantifierExecutor is intended for CRTP usage, when derived classes must inherit from "
        //    "QuantifierExecutor<Derived, ...>");

    public:
        //static_assert(
        //    requires(const Derived& derived) {
        //        { derived.impl_quantifier_type() } -> std::same_as<QuantifierType>;
        //        { derived.impl_min_repetitions() } -> std::same_as<size_t>;
        //        { derived.impl_max_repetitions() } -> std::same_as<std::optional<size_t>>;
        //    },
        //    "Derived class must implement the QuantifierExecutor interface");

        explicit QuantifierExecutor(SpannedRef<Quantifier> part_ref) : m_part_ref(part_ref) {}

        explicit QuantifierExecutor(
            SpannedRef<Quantifier> part_ref,
            QuantifierDecision<Quantifier> decision)
            : m_part_ref(part_ref), m_last_decision(decision) {}

        bool execute(MatchingProcessState& process) {
            if (!m_last_decision.match_this) {
                if (num_repetitions() < min_repetitions()) {
                    finalize_exhausted(process);
                    return false;
                }
                finalize_success(process);
                return true;
            }

            auto max = max_repetitions();
            while (!max.has_value() || num_repetitions() < max.value()) {
                if (!match_inner(process)) {
                    if (num_repetitions_ok()) {
                        finalize_success(process);
                        return true;
                    } else {
                        finalize_too_few(process);
                        return false;
                    }
                }
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

        size_t num_repetitions() const {
            return m_last_decision.repetition_num;
        }

        bool num_repetitions_ok() const {
            auto min = min_repetitions();
            auto max = max_repetitions();
            auto actual = num_repetitions();
            return min <= actual && (!max.has_value() || actual <= max.value());
        }

        bool match_inner(MatchingProcessState& process) {
            const auto& inner = *m_part_ref.item().inner;
            auto ref = SpannedRef(inner.part(), inner.span());
            auto executor = PartExecutor<Part>(ref);
            auto ok = executor.execute(process);
            if (!ok) {
                return false;
            }
            ++m_last_decision.repetition_num;
            return true;
        }

        void finalize_exhausted(MatchingProcessState& process) const {
            // TODO.
        }

        void finalize_success(MatchingProcessState& process) const {
            // TODO.
        }

        void finalize_too_few(MatchingProcessState& process) const {
            // TODO.
        }

        SpannedRef<Quantifier> m_part_ref;
        QuantifierDecision<Quantifier> m_last_decision;
    };

    template <typename Quantifier>
    using QuantifierExecutorBase = QuantifierExecutor<PartExecutor<Quantifier>, Quantifier>;

    template <>
    class PartExecutor<part::Star> : public QuantifierExecutorBase<part::Star> {
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
    class PartExecutor<part::Plus> : public QuantifierExecutorBase<part::Plus> {
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
    class PartExecutor<part::Optional> : public QuantifierExecutorBase<part::Optional> {
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

    bool PartExecutor<Part>::execute(MatchingProcessState& process) const {
        return m_part_ref.item().visit([&](const auto& part) {
            auto ref = SpannedRef(part, m_part_ref.span());
            using PartT = std::remove_cvref_t<decltype(part)>;
            auto executor = PartExecutor<PartT>(ref);
            return executor.execute(process);
        });
    }

}  // namespace

Executor::Executor(const Regex& regex_ref) : m_regex_ref(regex_ref) {}

const Regex& Executor::regex_ref() const {
    return m_regex_ref.get();
}

MatchResult Executor::execute(const std::u32string_view& string) const {
    MatchResult result;
    // TODO.
    return result;
}

}  // namespace wr22::regex_executor::algorithms::backtracking
