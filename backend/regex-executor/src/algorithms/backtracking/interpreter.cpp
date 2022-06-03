// wr22
#include <wr22/regex_executor/algorithms/backtracking/decision_applicator.hpp>
#include <wr22/regex_executor/algorithms/backtracking/instruction.hpp>
#include <wr22/regex_executor/algorithms/backtracking/interpreter.hpp>
#include <wr22/regex_executor/algorithms/backtracking/match_failure.hpp>
#include <wr22/regex_executor/algorithms/backtracking/part_executor.hpp>
#include <wr22/regex_executor/algorithms/backtracking/step.hpp>
#include <wr22/regex_executor/utils/spanned_ref.hpp>
#include <wr22/regex_parser/regex/part.hpp>
#include <wr22/unicode/conversion.hpp>

// fmt
#include <fmt/compile.h>
#include <fmt/core.h>

namespace wr22::regex_executor::algorithms::backtracking {

Interpreter::Interpreter(const Regex& regex, const std::u32string_view& string_ref)
    : m_string_ref(string_ref),
      m_current_state(InterpreterState{
          .captures =
              Captures{
                  .whole =
                      Capture{
                          .string_span = regex_parser::span::Span::make_empty(0),
                      },
              },
      }) {
    auto ref = utils::SpannedRef<regex_parser::regex::Part>(
        regex.root_part().part(),
        regex.root_part().span());
    m_current_state.instructions.push_back(instruction::Execute{ref});
}

std::optional<char32_t> Interpreter::current_char() const {
    if (m_current_state.cursor >= m_string_ref.size()) {
        return std::nullopt;
    }
    return m_string_ref[m_current_state.cursor];
}

void Interpreter::advance() {
    ++m_current_state.cursor;
}

DecisionRef Interpreter::add_decision(
    Decision decision,
    utils::SpannedRef<regex_parser::regex::Part> decision_making_part) {
    auto index = m_decision_snapshots.size();
    auto snapshot = InterpreterStateSnapshot{
        .state = m_current_state,
        .decision_making_part = decision_making_part,
        .before_step = m_steps.size(),
    };
    auto decision_snapshot = DecisionSnapshot{
        .snapshot = std::move(snapshot),
        .decision = std::move(decision),
    };
    m_decision_snapshots.push_back(std::move(decision_snapshot));
    return DecisionRef{.index = index};
}

const DecisionSnapshot& Interpreter::decision_snapshot_at(DecisionRef ref) const {
    return m_decision_snapshots.at(ref.index);
}

DecisionSnapshot& Interpreter::decision_snapshot_at(DecisionRef ref) {
    return m_decision_snapshots.at(ref.index);
}

void Interpreter::restore_from_snapshot(InterpreterStateSnapshot snapshot) {
    add_step(step::Backtrack{
        .string_pos = cursor(),
        .continue_after_step = snapshot.before_step - 1,
    });
    m_current_state = std::move(snapshot.state);
}

void Interpreter::push_mini_snapshot() {
    m_mini_snapshots.push(InterpreterStateMiniSnapshot{
        .cursor = cursor(),
        .before_step = m_steps.size(),
    });
}

void Interpreter::pop_mini_snapshot() {
    m_mini_snapshots.pop();
}

std::optional<std::reference_wrapper<const InterpreterStateMiniSnapshot>> Interpreter::
    last_mini_snapshot() const {
    if (m_mini_snapshots.empty()) {
        return std::nullopt;
    } else {
        return std::cref(m_mini_snapshots.top());
    }
}

void Interpreter::add_instruction(Instruction instruction) {
    m_current_state.instructions.push_back(std::move(instruction));
}

void Interpreter::add_step(Step step) {
    m_steps.push_back(std::move(step));
}

void Interpreter::push_counter(size_t initial) {
    m_current_state.counters.push_back(initial);
}

void Interpreter::pop_counter() {
    m_current_state.counters.pop_back();
}

void Interpreter::push_error_hook(ErrorHook hook) {
    m_current_state.error_hooks.push(std::move(hook));
}

void Interpreter::pop_error_hook() {
    m_current_state.error_hooks.pop();
}

size_t& Interpreter::counter_at_offset(size_t offset) {
    auto index = parse_counter_offset(offset);
    return m_current_state.counters.at(index);
}

size_t Interpreter::counter_at_offset(size_t offset) const {
    auto index = parse_counter_offset(offset);
    return m_current_state.counters.at(index);
}

size_t Interpreter::cursor() const {
    return m_current_state.cursor;
}

bool Interpreter::finished() const {
    return m_current_state.instructions.empty();
}

void Interpreter::run_instruction() {
    auto instruction = std::move(m_current_state.instructions.back());
    m_current_state.instructions.pop_back();

    auto ok = instruction.visit(
        [this](const instruction::AddStep& instruction) {
            add_step(std::move(instruction.step));
            return true;
        },
        [this](const instruction::Execute& instruction) {
            if (instruction.forced_decision.has_value()) {
                return instruction.forced_decision.value().visit(
                    [this, part = instruction.part](const auto& decision) {
                        using DecisionType = std::remove_cvref_t<decltype(decision)>;
                        using ApplicatorType = DecisionApplicator<DecisionType>;
                        using PartType = typename DecisionType::PartType;
                        auto specific_part = utils::SpannedRef<PartType>(
                            std::get<PartType>(part.item().as_variant()),
                            part.span());
                        auto executor = ApplicatorType::construct_executor(
                            specific_part,
                            part,
                            decision);
                        return executor.execute(*this);
                    });
            } else {
                auto executor = PartExecutor(instruction.part);
                return executor.execute(*this);
            }
        },
        [this](const instruction::Run& instruction) {
            instruction(*this);
            return true;
        });

    if (ok) {
        return;
    }

    // Reconsider the decision if matching failed.

    // Run the topmost error hook if any.
    if (!m_current_state.error_hooks.empty()) {
        const auto& hook = m_current_state.error_hooks.top();
        hook(*this);
        // No need to pop the hook, as `<DecisionType>::reconsider()` will take care of it.
    }

    // If we can reconsider a decision we have made earlier, do so.
    // We might need to go arbitrarily deep into the decision stack, since some decisions
    // we have made may have no more options remaining.
    while (true) {
        if (m_decision_snapshots.empty()) {
            throw MatchFailure{};
        }

        auto last_decision_snapshot = std::move(m_decision_snapshots.back());
        m_decision_snapshots.pop_back();
        auto decision_making_part = last_decision_snapshot.snapshot.decision_making_part;
        auto has_reconsidered = last_decision_snapshot.decision.visit(
            [this, snapshot = std::move(last_decision_snapshot.snapshot)](auto& decision) {
                using DecisionType = std::decay_t<decltype(decision)>;

                auto decision_making_part = snapshot.decision_making_part;
                auto new_decision = decision.reconsider(*this, std::move(snapshot));
                if (!new_decision.has_value()) {
                    // Options exhausted for this decision, try the next one.
                    using ApplicatorType = DecisionApplicator<DecisionType>;
                    auto specific_ref = utils::SpannedRef(
                        std::get<typename ApplicatorType::PartType>(
                            decision_making_part.item().as_variant()),
                        decision_making_part.span());
                    ApplicatorType::finalize_exhausted(decision, specific_ref, *this);
                    return false;
                }
                decision = std::move(new_decision.value());
                return true;
            });

        if (has_reconsidered) {
            m_current_state.instructions.push_back(instruction::Execute{
                .part = decision_making_part,
                .forced_decision = std::move(last_decision_snapshot.decision),
            });
            break;
        }
    }
}

void Interpreter::finalize() {
    add_step(step::End{
        .string_pos = cursor(),
        .result = step::End::Success{},
    });
    m_current_state.captures.whole.string_span = regex_parser::span::Span::make_from_positions(
        0,
        cursor());
}

void Interpreter::finalize_error() {
    add_step(step::End{
        .string_pos = cursor(),
        .result = step::End::Failure{},
    });
}

void Interpreter::add_indexed_capture(Capture capture) {
    auto index = m_current_state.capture_counter;
    m_current_state.captures.indexed.insert({index, capture});
    ++m_current_state.capture_counter;
}

void Interpreter::add_named_capture(std::string_view name, Capture capture) {
    m_current_state.captures.named.insert_or_assign(name, capture);
}

size_t Interpreter::parse_counter_offset(size_t offset) const {
    auto size = m_current_state.counters.size();
    if (offset >= size) {
        throw std::out_of_range(fmt::format(FMT_STRING("No counter at offset {} exists"), offset));
    }
    return size - offset - 1;
}

std::vector<Step> Interpreter::into_steps() && {
    return std::move(m_steps);
}

const InterpreterState& Interpreter::current_state() const {
    return m_current_state;
}

InterpreterState& Interpreter::current_state() {
    return m_current_state;
}

}  // namespace wr22::regex_executor::algorithms::backtracking
