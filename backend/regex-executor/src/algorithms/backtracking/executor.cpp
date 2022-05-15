// wr22
#include <variant>
#include <wr22/regex_executor/algorithms/backtracking/executor.hpp>
#include <wr22/regex_executor/algorithms/backtracking/step.hpp>
#include <wr22/regex_parser/regex/character_class_data.hpp>
#include <wr22/regex_parser/regex/part.hpp>
#include <wr22/utils/adt.hpp>

// stl
#include <functional>
#include <optional>
#include <stack>
#include <stdexcept>
#include <string_view>
#include <type_traits>

// fmt
#include <fmt/compile.h>
#include <fmt/core.h>

namespace wr22::regex_executor::algorithms::backtracking {

namespace {
    namespace part = regex_parser::regex::part;
    using regex_parser::regex::CharacterClassData;
    using regex_parser::regex::Part;
    using regex_parser::span::Span;

    class Interpreter;

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
        std::optional<size_t> num_repetitions;
    };

    using Decision = utils::Adt<
        AlternativesDecision,
        QuantifierDecision<part::Star>,
        QuantifierDecision<part::Plus>,
        QuantifierDecision<part::Optional>>;

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

    namespace instruction {
        struct Execute {
            SpannedRef<Part> part;
        };

        struct AddStep {
            Step step;
        };

        struct Run {
            using Context = utils::Adt<
                std::monostate,
                SpannedRef<part::Star>,
                SpannedRef<part::Plus>,
                SpannedRef<part::Optional>>;
            using Fn = void (*)(const Context& ctx, Interpreter& interpreter);
            Context ctx;
            Fn fn;

            void operator()(Interpreter& interpreter) const {
                fn(ctx, interpreter);
            }
        };

        using Adt = utils::Adt<Execute, AddStep, Run>;
    };  // namespace instruction

    using Instruction = instruction::Adt;

    struct DecisionRef {
        size_t index;
    };

    struct ErrorHook {
        using Context = std::variant<std::monostate, DecisionRef>;
        using Fn = void (*)(const Context& context, Interpreter& interpreter);
        Context ctx;
        Fn fn;

        void operator()(Interpreter& interpreter) const {
            fn(ctx, interpreter);
        }
    };

    struct InterpreterState {
        // TODO: captures, if they belong here.
        size_t cursor = 0;
        std::stack<Instruction> instructions;
        std::stack<ErrorHook> error_hooks;
        std::vector<size_t> counters;
    };

    struct InterpreterStateSnapshot {
        InterpreterState state;
        size_t before_decision;
        size_t before_step;
    };

    template <>
    class PartExecutor<Part> {
    public:
        explicit PartExecutor(SpannedRef<Part> part_ref) : m_part_ref(part_ref) {}

        bool execute(Interpreter& interpreter) const;

    private:
        SpannedRef<Part> m_part_ref;
    };

    class Interpreter {
    public:
        explicit Interpreter(const std::u32string_view& string_ref) : m_string_ref(string_ref) {}

        std::optional<char32_t> current_char() const {
            if (m_current_state.cursor >= m_string_ref.size()) {
                return std::nullopt;
            }
            return m_string_ref[m_current_state.cursor];
        }

        void advance() {
            ++m_current_state.cursor;
        }

        void make_snapshot() {
            auto snapshot = InterpreterStateSnapshot{
                .state = m_current_state,
                .before_decision = m_decision_stack.size(),
                .before_step = m_steps.size(),
            };
            m_snapshots.push(std::move(snapshot));
        }

        DecisionRef add_decision(Decision decision) {
            auto index = m_decision_stack.size();
            m_decision_stack.push_back(std::move(decision));
            return DecisionRef{.index = index};
        }

        const Decision& decision_at(DecisionRef ref) const {
            return m_decision_stack.at(ref.index);
        }

        Decision& decision_at(DecisionRef ref) {
            return m_decision_stack.at(ref.index);
        }

        void add_instruction(Instruction instruction) {
            m_current_state.instructions.push(std::move(instruction));
        }

        void add_step(Step step) {
            m_steps.push_back(std::move(step));
        }

        void push_counter(size_t initial) {
            m_current_state.counters.push_back(initial);
        }

        void pop_counter() {
            m_current_state.counters.pop_back();
        }

        void push_error_hook(ErrorHook hook) {
            m_current_state.error_hooks.push(std::move(hook));
        }

        void pop_error_hook() {
            m_current_state.error_hooks.pop();
        }

        size_t& counter_at_offset(size_t offset) {
            auto index = parse_counter_offset(offset);
            return m_current_state.counters.at(index);
        }

        size_t counter_at_offset(size_t offset) const {
            auto index = parse_counter_offset(offset);
            return m_current_state.counters.at(index);
        }

        size_t cursor() const {
            return m_current_state.cursor;
        }

        bool finished() const {
            return m_current_state.instructions.empty();
        }

        void run_instruction() {
            auto instruction = std::move(m_current_state.instructions.top());
            m_current_state.instructions.pop();
            auto ok = instruction.visit(
                [this](const instruction::AddStep& instruction) {
                    add_step(std::move(instruction.step));
                    return true;
                },
                [this](const instruction::Execute& instruction) {
                    auto executor = PartExecutor<Part>(instruction.part);
                    return executor.execute(*this);
                },
                [this](const instruction::Run& instruction) {
                    instruction(*this);
                    return true;
                });
            if (!ok) {
                // TODO: reconsider the decision.
            }
        }

    private:
        size_t parse_counter_offset(size_t offset) const {
            auto size = m_current_state.counters.size();
            if (offset >= size) {
                throw std::out_of_range(
                    fmt::format(FMT_STRING("No counter at offset {} exists"), offset));
            }
            return size - offset;
        }

        std::u32string_view m_string_ref;
        InterpreterState m_current_state;
        // Not using std::stack to be able to truncate to a specific size.
        std::vector<Decision> m_decision_stack;
        std::stack<InterpreterStateSnapshot> m_snapshots;
        std::vector<Step> m_steps;
    };

    template <>
    class PartExecutor<part::Empty> {
    public:
        explicit PartExecutor([[maybe_unused]] SpannedRef<part::Empty> part_ref) {}

        bool execute([[maybe_unused]] Interpreter& interpreter) const {
            return true;
        }
    };

    template <>
    class PartExecutor<part::Literal> {
    public:
        explicit PartExecutor(SpannedRef<part::Literal> part_ref) : m_part_ref(part_ref) {}

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
    class PartExecutor<part::Wildcard> {
    public:
        explicit PartExecutor(SpannedRef<part::Wildcard> part_ref) : m_part_ref(part_ref) {}

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
    class PartExecutor<part::Sequence> {
    public:
        explicit PartExecutor(SpannedRef<part::Sequence> part_ref) : m_part_ref(part_ref) {}

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
    class PartExecutor<part::Group> {
    public:
        explicit PartExecutor(SpannedRef<part::Group> part_ref) : m_part_ref(part_ref) {}

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
    class PartExecutor<part::Alternatives> {
    public:
        explicit PartExecutor(SpannedRef<part::Alternatives> part_ref)
            : m_part_ref(part_ref), m_decision{.part_ref = part_ref.item()} {}

        explicit PartExecutor(SpannedRef<part::Alternatives> part_ref, AlternativesDecision decision)
            : m_part_ref(part_ref), m_decision(std::move(decision)) {}

        bool execute(Interpreter& interpreter) const {
            interpreter.make_snapshot();
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
    class PartExecutor<part::CharacterClass> {
    public:
        explicit PartExecutor(SpannedRef<part::CharacterClass> part_ref) : m_part_ref(part_ref) {}

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

                // (3) Uninstall the error hook.
                interpreter.add_instruction(instruction::Run{
                    .ctx = std::monostate{},
                    .fn = []([[maybe_unused]] const instruction::Run::Context& ctx,
                             Interpreter& interpreter) { interpreter.pop_error_hook(); },
                });

                // (2) Run the greedy walk process.
                interpreter.add_instruction(instruction::Run{
                    .ctx = m_part_ref,
                    .fn = greedy_walk_run_func,
                });

                // (1) Install the error hook.
                interpreter.add_instruction(instruction::Run{
                    .ctx = std::monostate{},
                    .fn =
                        []([[maybe_unused]] const instruction::Run::Context& ctx,
                           Interpreter& interpreter) {
                            interpreter.push_error_hook(ErrorHook{
                                .ctx = std::monostate{},
                                .fn =
                                    [](const ErrorHook::Context& ctx, Interpreter& interpreter) {
                                        auto real_num_repetitions = interpreter.counter_at_offset(
                                            0);
                                        auto decision_ref = std::get<DecisionRef>(ctx);
                                        // Set the number of repetitions to the actual one in the
                                        // decision made earlier.
                                        std::get<QuantifierDecision<Quantifier>>(
                                            interpreter.decision_at(decision_ref).as_variant())
                                            .num_repetitions = real_num_repetitions;
                                        // Pop the repetition counter.
                                        interpreter.pop_counter();
                                    },
                            });
                        },
                });
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

        static void greedy_walk_run_func(
            const instruction::Run::Context& ctx,
            Interpreter& interpreter) {
            // (3) Run quasi-recursively.
            interpreter.add_instruction(instruction::Run{
                .ctx = ctx,
                .fn = greedy_walk_run_func,
            });
            // (2) Increment the repetition counter.
            interpreter.add_instruction(instruction::Run{
                .ctx = std::monostate{},
                .fn =
                    []([[maybe_unused]] const instruction::Run::Context& ctx,
                       Interpreter& interpreter) {
                        auto& current_num_repetitions = interpreter.counter_at_offset(0);
                        ++current_num_repetitions;
                    },
            });
            // (1) Match one item.
            const auto& part = std::get<SpannedRef<Quantifier>>(ctx.as_variant());
            const auto& inner = *part.item().inner;
            auto ref = SpannedRef<Part>(inner.part(), inner.span());
            interpreter.add_instruction(instruction::Execute{ref});
        }

        SpannedRef<Quantifier> m_part_ref;
        QuantifierDecision<Quantifier> m_decision;
        size_t m_already_matched;
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

    bool PartExecutor<Part>::execute(Interpreter& interpreter) const {
        return m_part_ref.item().visit([&](const auto& part) {
            auto ref = SpannedRef(part, m_part_ref.span());
            using PartT = std::remove_cvref_t<decltype(part)>;
            auto executor = PartExecutor<PartT>(ref);
            return executor.execute(interpreter);
        });
    }

}  // namespace

Executor::Executor(const Regex& regex_ref) : m_regex_ref(regex_ref) {}

const Regex& Executor::regex_ref() const {
    return m_regex_ref.get();
}

MatchResult Executor::execute(const std::u32string_view& string) const {
    Interpreter interpreter(string);
    while (!interpreter.finished()) {
        interpreter.run_instruction();
    }
}

}  // namespace wr22::regex_executor::algorithms::backtracking
