// wr22
#include <wr22/regex_executor/algorithms/backtracking/interpreter.hpp>
#include <wr22/regex_executor/algorithms/backtracking/part_executor.hpp>
#include <wr22/regex_parser/regex/part.hpp>

// fmt
#include <fmt/compile.h>
#include <fmt/core.h>

namespace wr22::regex_executor::algorithms::backtracking {

using wr22::regex_parser::regex::Part;

Interpreter::Interpreter(const std::u32string_view& string_ref) : m_string_ref(string_ref) {}

std::optional<char32_t> Interpreter::current_char() const {
    if (m_current_state.cursor >= m_string_ref.size()) {
        return std::nullopt;
    }
    return m_string_ref[m_current_state.cursor];
}

void Interpreter::advance() {
    ++m_current_state.cursor;
}

void Interpreter::make_snapshot() {
    auto snapshot = InterpreterStateSnapshot{
        .state = m_current_state,
        .before_decision = m_decision_stack.size(),
        .before_step = m_steps.size(),
    };
    m_snapshots.push(std::move(snapshot));
}

DecisionRef Interpreter::add_decision(Decision decision) {
    auto index = m_decision_stack.size();
    m_decision_stack.push_back(std::move(decision));
    return DecisionRef{.index = index};
}

const Decision& Interpreter::decision_at(DecisionRef ref) const {
    return m_decision_stack.at(ref.index);
}

Decision& Interpreter::decision_at(DecisionRef ref) {
    return m_decision_stack.at(ref.index);
}

void Interpreter::add_instruction(Instruction instruction) {
    m_current_state.instructions.push(std::move(instruction));
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
        if (m_decision_stack.empty()) {
            // TODO: signal about failed matching.
            std::terminate();
        }

        auto& last_decision = m_decision_stack.back();
        auto has_reconsidered = last_decision.visit([this](auto& decision) {
            auto new_decision = decision.reconsider(*this);
            if (!new_decision.has_value()) {
                // Options exhausted for this decision, try the next one.
                return false;
            }
            decision = std::move(new_decision.value());
        });

        if (has_reconsidered) {
            break;
        }
    }
}

size_t Interpreter::parse_counter_offset(size_t offset) const {
    auto size = m_current_state.counters.size();
    if (offset >= size) {
        throw std::out_of_range(fmt::format(FMT_STRING("No counter at offset {} exists"), offset));
    }
    return size - offset;
}

}  // namespace wr22::regex_executor::algorithms::backtracking
