// wr22
#include <string_view>
#include <type_traits>
#include <wr22/regex_executor/algorithms/backtracking/executor.hpp>
#include <wr22/regex_executor/algorithms/backtracking/step.hpp>
#include <wr22/regex_parser/regex/part.hpp>
#include <wr22/utils/adt.hpp>

// stl
#include <functional>
#include <optional>
#include <stack>

namespace wr22::regex_executor::algorithms::backtracking {

namespace {
    namespace part = regex_parser::regex::part;
    using regex_parser::regex::Part;
    using regex_parser::regex::SpannedPart;
    using regex_parser::span::Span;

    template <typename T>
    class PartExecutor {};

    struct AlternativesDecision {
        using PartType = part::Alternatives;

        // SAFETY: comes from borrowed `regex`, safe to keep as long
        // as the `Executor` is alive, provided its safety assumptions
        // are met.
        std::reference_wrapper<const part::Alternatives> part_ref;
        size_t decision_index;
    };

    template <typename PartT>
    struct QuantifierDecision {
        using PartType = PartT;
        size_t num_repetitions;
    };

    struct MatchingState {
        // TODO: captures.
        size_t cursor = 0;
    };

    using DecisionAdt = utils::Adt<AlternativesDecision>;
    struct Decision : public DecisionAdt {
        using DecisionAdt::DecisionAdt;
    };

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
                auto item_executor = PartExecutor(ref);
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
            auto inner_executor = PartExecutor(inner_ref);
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
    class PartExecutor<Part> {
    public:
        explicit PartExecutor(SpannedRef<Part> part_ref) : m_part_ref(part_ref) {}

        bool execute(MatchingProcessState& process) const {
            m_part_ref.item().visit([&](const auto& part) {
                auto ref = SpannedRef(part, m_part_ref.span());
                using PartT = std::remove_cvref_t<decltype(part)>;
                auto executor = PartExecutor<PartT>(ref);
                return executor.execute(process);
            });
        }

    private:
        SpannedRef<Part> m_part_ref;
    };
}  // namespace

Executor::Executor(const Regex& regex_ref) : m_regex_ref(regex_ref) {}

const Regex& Executor::regex_ref() const {
    return m_regex_ref.get();
}

MatchResult Executor::execute(const std::u32string_view& string) const {
    MatchResult result;
}

}  // namespace wr22::regex_executor::algorithms::backtracking
