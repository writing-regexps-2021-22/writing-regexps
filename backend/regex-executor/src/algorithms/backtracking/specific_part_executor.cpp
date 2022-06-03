// wr22
#include "wr22/regex_executor/utils/spanned_ref.hpp"
#include <wr22/regex_executor/algorithms/backtracking/instruction.hpp>
#include <wr22/regex_executor/algorithms/backtracking/specific_part_executor.hpp>
#include <wr22/regex_executor/algorithms/backtracking/step.hpp>

namespace wr22::regex_executor::algorithms::backtracking {

namespace part = regex_parser::regex::part;

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

SpecificPartExecutor<part::Empty>::SpecificPartExecutor(
    [[maybe_unused]] utils::SpannedRef<part::Empty> part_ref) {}

bool SpecificPartExecutor<part::Empty>::execute([[maybe_unused]] Interpreter& interpreter) const {
    return true;
}

SpecificPartExecutor<part::Literal>::SpecificPartExecutor(utils::SpannedRef<part::Literal> part_ref)
    : m_part_ref(part_ref) {}

bool SpecificPartExecutor<part::Literal>::execute(Interpreter& interpreter) const {
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
                .string_span = regex_parser::span::Span::make_single_position(interpreter.cursor()),
            },
    });
    interpreter.advance();
    return true;
}

SpecificPartExecutor<part::Wildcard>::SpecificPartExecutor(
    utils::SpannedRef<part::Wildcard> part_ref)
    : m_part_ref(part_ref) {}

bool SpecificPartExecutor<part::Wildcard>::execute(Interpreter& interpreter) const {
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
                .string_span = regex_parser::span::Span::make_single_position(interpreter.cursor()),
            },
    });
    interpreter.advance();
    return true;
}

SpecificPartExecutor<part::Sequence>::SpecificPartExecutor(
    utils::SpannedRef<part::Sequence> part_ref)
    : m_part_ref(part_ref) {}

bool SpecificPartExecutor<part::Sequence>::execute(Interpreter& interpreter) const {
    // We could have used `std::views::reverse` from C++20, but
    // clang doesn't seem to support it properly (at least the libstdc++ implementation).
    const auto& items = m_part_ref.item().items;
    for (auto it = items.rbegin(); it != items.rend(); ++it) {
        const auto& spanned_part = *it;
        auto ref = utils::SpannedRef(spanned_part.part(), spanned_part.span());
        interpreter.add_instruction(instruction::Execute{.part = ref});
    }
    return true;
}

SpecificPartExecutor<part::Group>::SpecificPartExecutor(utils::SpannedRef<part::Group> part_ref)
    : m_part_ref(part_ref) {}

bool SpecificPartExecutor<part::Group>::execute(Interpreter& interpreter) const {
    interpreter.add_instruction(instruction::Run{
        .ctx = std::monostate{},
        .fn =
            []([[maybe_unused]] const instruction::Run::Context& ctx, Interpreter& interpreter) {
                interpreter.add_step(step::EndGroup{
                    .string_pos = interpreter.cursor(),
                });
            },
    });
    const auto& inner = *m_part_ref.item().inner;
    auto inner_ref = utils::SpannedRef(inner.part(), inner.span());
    interpreter.add_instruction(instruction::Execute{inner_ref});
    interpreter.add_instruction(instruction::AddStep{step::BeginGroup{
        .regex_span = m_part_ref.span(),
        .string_pos = interpreter.cursor(),
    }});
    return true;
}

SpecificPartExecutor<part::Alternatives>::SpecificPartExecutor(
    utils::SpannedRef<part::Alternatives> part_ref,
    utils::SpannedRef<regex_parser::regex::Part> part_var_ref)
    : m_part_ref(part_ref), m_part_var_ref(part_var_ref), m_decision{.part_ref = part_ref.item()} {}

SpecificPartExecutor<part::Alternatives>::SpecificPartExecutor(
    utils::SpannedRef<part::Alternatives> part_ref,
    utils::SpannedRef<regex_parser::regex::Part> part_var_ref,
    AlternativesDecision decision)
    : m_part_ref(part_ref), m_part_var_ref(part_var_ref), m_decision(std::move(decision)) {}

bool SpecificPartExecutor<part::Alternatives>::execute(Interpreter& interpreter) {
    m_decision.initial_string_pos = interpreter.cursor();
    if (m_decision.decision_index == 0) {
        interpreter.add_step(step::MatchAlternatives{
            .regex_span = m_part_ref.span(),
            .string_pos = interpreter.cursor(),
        });
    }
    interpreter.add_decision(m_decision, m_part_var_ref);
    const auto& alternative = m_part_ref.item().alternatives.at(m_decision.decision_index);
    auto ref = utils::SpannedRef(alternative.part(), alternative.span());

    // (2) Finalize matching as a step.
    auto step_base = step::FinishAlternatives{
        .regex_span = m_part_ref.span(),
        .result =
            step::FinishAlternatives::Success{
                .string_span = regex_parser::span::Span::make_empty(
                    m_decision.initial_string_pos.value()),
                .alternative_chosen = m_decision.decision_index,
            },
    };
    interpreter.add_instruction(instruction::Run{
        .ctx = std::move(step_base),
        .fn =
            [](const instruction::Run::Context& ctx, Interpreter& interpreter) {
                auto step = std::get<Step>(ctx.as_variant());
                auto& result = std::get<step::FinishAlternatives>(step.as_variant()).result;
                auto& span = std::get<step::FinishAlternatives::Success>(result.as_variant())
                                 .string_span;
                span = regex_parser::span::Span::make_from_positions(
                    span.begin(),
                    interpreter.cursor());
                interpreter.add_step(std::move(step));
            },
    });
    // (1) Match according to our decision.
    interpreter.add_instruction(instruction::Execute{ref});
    return true;
}

SpecificPartExecutor<part::CharacterClass>::SpecificPartExecutor(
    utils::SpannedRef<part::CharacterClass> part_ref)
    : m_part_ref(part_ref) {}

bool SpecificPartExecutor<part::CharacterClass>::execute(Interpreter& interpreter) const {
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
                .string_span = regex_parser::span::Span::make_single_position(interpreter.cursor()),
            },
    });
    interpreter.advance();
    return true;
}

template <typename Derived, typename Quantifier>
QuantifierExecutor<Derived, Quantifier>::QuantifierExecutor(
    utils::SpannedRef<Quantifier> part_ref,
    utils::SpannedRef<regex_parser::regex::Part> part_var_ref)
    : m_part_ref(part_ref), m_part_var_ref(part_var_ref), m_stop_immediately(false) {}

template <typename Derived, typename Quantifier>
QuantifierExecutor<Derived, Quantifier>::QuantifierExecutor(
    utils::SpannedRef<Quantifier> part_ref,
    utils::SpannedRef<regex_parser::regex::Part> part_var_ref,
    [[maybe_unused]] StopImmediatelyTag tag)
    : m_part_ref(part_ref), m_part_var_ref(part_var_ref), m_stop_immediately(true) {}

template <typename Derived, typename Quantifier>
bool QuantifierExecutor<Derived, Quantifier>::execute(Interpreter& interpreter) {
    std::cout << "  ~ execute quantifier" << std::endl;

    if (m_stop_immediately) {
        return true;
    }

    // (1) Add initial step and set up counters.
    interpreter.add_step(step::MatchQuantifier{
        .regex_span = m_part_ref.span(),
        .string_pos = interpreter.cursor(),
        .quantifier_type = quantifier_type(),
    });
    // Counter at offset 1: initial cursor position (read-only).
    interpreter.push_counter(interpreter.cursor());
    // Counter at offset 0: current number of repetitions (updated at each step).
    interpreter.push_counter(0);

    // (3) Finalize: pop counters and add final step.
    interpreter.add_instruction(instruction::Run{
        .ctx = std::make_pair(m_part_ref, m_part_var_ref),
        .fn = finalize_run_func,
    });

    // (2) Run the greedy walk process.
    interpreter.add_instruction(instruction::Run{
        .ctx = std::make_pair(m_part_ref, m_part_var_ref),
        .fn = greedy_walk_run_func,
    });

    return true;
}

template <typename Derived, typename Quantifier>
size_t QuantifierExecutor<Derived, Quantifier>::min_repetitions() {
    return Derived::impl_min_repetitions();
}

template <typename Derived, typename Quantifier>
std::optional<size_t> QuantifierExecutor<Derived, Quantifier>::max_repetitions() {
    return Derived::impl_max_repetitions();
}

template <typename Derived, typename Quantifier>
QuantifierType QuantifierExecutor<Derived, Quantifier>::quantifier_type() {
    return Derived::impl_quantifier_type();
}

template <typename Derived, typename Quantifier>
bool QuantifierExecutor<Derived, Quantifier>::num_repetitions_ok(size_t num_repetitions) {
    auto min = min_repetitions();
    auto max = max_repetitions();
    return min <= num_repetitions && (!max.has_value() || num_repetitions <= max.value());
}

template <typename Derived, typename Quantifier>
void QuantifierExecutor<Derived, Quantifier>::finalize_run_func(
    const instruction::Run::Context& ctx,
    Interpreter& interpreter) {
    auto num_repetitions = interpreter.counter_at_offset(0);
    auto span_begin = interpreter.counter_at_offset(1);
    auto span = regex_parser::span::Span::make_from_positions(span_begin, interpreter.cursor());

    interpreter.add_step(step::FinishQuantifier{
        .quantifier_type = quantifier_type(),
        .regex_span = std::get<std::pair<
            utils::SpannedRef<Quantifier>,
            utils::SpannedRef<regex_parser::regex::Part>>>(ctx.as_variant())
                          .first.span(),
        .result =
            step::FinishQuantifier::Success{
                .string_span = span,
                .num_repetitions = num_repetitions,
            },
    });
    interpreter.pop_counter();
    interpreter.pop_counter();
}

template <typename Derived, typename Quantifier>
void QuantifierExecutor<Derived, Quantifier>::greedy_walk_run_func(
    const instruction::Run::Context& ctx,
    Interpreter& interpreter) {
    std::cout << "  ~ greedy_walk_run_func" << std::endl;

    // (1) Read current state and compute the next one or stop.
    auto num_repetitions_so_far = interpreter.counter_at_offset(0);
    auto next_num_repetitions = num_repetitions_so_far + 1;
    if (auto max = max_repetitions(); max.has_value() && next_num_repetitions > max.value()) {
        // Stop just before the maximum allowable number of repetitions is exceeded.
        return;
    }
    auto can_go_back = next_num_repetitions > min_repetitions();

    const auto& [part, part_var] = std::get<
        std::pair<utils::SpannedRef<Quantifier>, utils::SpannedRef<regex_parser::regex::Part>>>(
        ctx.as_variant());

    // (2) Make a decision to continue matching.
    interpreter.add_decision(
        QuantifierDecision<Quantifier>{
            .stop_here = false,
            .can_go_back = can_go_back,
        },
        part_var);

    // (5) Run quasi-recursively.
    interpreter.add_instruction(instruction::Run{
        .ctx = ctx,
        .fn = greedy_walk_run_func,
    });

    // (4) Increment the repetition counter.
    interpreter.add_instruction(instruction::Run{
        .ctx = std::monostate{},
        .fn =
            []([[maybe_unused]] const instruction::Run::Context& ctx, Interpreter& interpreter) {
                std::cout << "  ~ greedy_walk_run_func: increment" << std::endl;
                auto& current_num_repetitions = interpreter.counter_at_offset(0);
                ++current_num_repetitions;
            },
    });

    // (3) Match one item
    const auto& inner = *part.item().inner;
    auto ref = utils::SpannedRef<regex_parser::regex::Part>(inner.part(), inner.span());
    interpreter.add_instruction(instruction::Execute{ref});
}

template class QuantifierExecutor<SpecificPartExecutor<part::Star>, part::Star>;
template class QuantifierExecutor<SpecificPartExecutor<part::Plus>, part::Plus>;
template class QuantifierExecutor<SpecificPartExecutor<part::Optional>, part::Optional>;

template <typename Quantifier>
using QuantifierExecutorBase = QuantifierExecutor<SpecificPartExecutor<Quantifier>, Quantifier>;

QuantifierType SpecificPartExecutor<part::Star>::impl_quantifier_type() {
    return QuantifierType::Star;
}

size_t SpecificPartExecutor<part::Star>::impl_min_repetitions() {
    return 0;
}

std::optional<size_t> SpecificPartExecutor<part::Star>::impl_max_repetitions() {
    return std::nullopt;
}

QuantifierType SpecificPartExecutor<part::Plus>::impl_quantifier_type() {
    return QuantifierType::Plus;
}

size_t SpecificPartExecutor<part::Plus>::impl_min_repetitions() {
    return 1;
}

std::optional<size_t> SpecificPartExecutor<part::Plus>::impl_max_repetitions() {
    return std::nullopt;
}

QuantifierType SpecificPartExecutor<part::Optional>::impl_quantifier_type() {
    return QuantifierType::Optional;
}

size_t SpecificPartExecutor<part::Optional>::impl_min_repetitions() {
    return 0;
}

std::optional<size_t> SpecificPartExecutor<part::Optional>::impl_max_repetitions() {
    return 1;
}

}  // namespace wr22::regex_executor::algorithms::backtracking
