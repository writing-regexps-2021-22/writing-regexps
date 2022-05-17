// wr22
#include <wr22/regex_executor/algorithms/backtracking/instruction.hpp>
#include <wr22/regex_executor/algorithms/backtracking/interpreter.hpp>
#include <wr22/regex_executor/algorithms/backtracking/part_executor.hpp>
#include <wr22/regex_executor/algorithms/backtracking/step.hpp>
#include <wr22/regex_executor/utils/spanned_ref.hpp>
#include <wr22/regex_parser/regex/part.hpp>

// fmt
#include <fmt/compile.h>
#include <fmt/core.h>

#include <iostream>

namespace wr22::regex_executor::algorithms::backtracking {

Interpreter::Interpreter(const Regex& regex, const std::u32string_view& string_ref)
    : m_string_ref(string_ref) {
    auto ref = utils::SpannedRef<regex_parser::regex::Part>(
        regex.root_part().part(),
        regex.root_part().span());
    m_current_state.instructions.push(instruction::Execute{ref});
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

DecisionRef Interpreter::add_decision(Decision decision) {
    auto index = m_decision_snapshots.size();
    auto snapshot = InterpreterStateSnapshot{
        .state = m_current_state,
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
    std::cout << "==> Run instruction [at " << cursor() << "]" << std::endl;
    std::cout << " ### Counters: ";
    for (auto x : m_current_state.counters) {
        std::cout << x << ", ";
    }
    std::cout << "<++>" << std::endl;
    auto ok = instruction.visit(
        [this](const instruction::AddStep& instruction) {
            std::cout << " -> add step: " << nlohmann::json(instruction.step).dump() << std::endl;
            add_step(std::move(instruction.step));
            return true;
        },
        [this](const instruction::Execute& instruction) {
            std::cout << " -> execute part: " << instruction.part.span() << std::endl;
            if (instruction.forced_decision.has_value()) {
                return instruction.forced_decision.value().visit(
                    [this, part = instruction.part](const auto& decision) {
                        auto executor = decision.construct_executor(part);
                        return executor.execute(*this);
                    });
            } else {
                auto executor = PartExecutor(instruction.part);
                return executor.execute(*this);
            }
        },
        [this](const instruction::Run& instruction) {
            std::cout << " -> run function " << (void*)instruction.fn << std::endl;
            instruction(*this);
            return true;
        });

    if (ok) {
        std::cout << " -> ok" << std::endl;
        return;
    }
    std::cout << " -> not ok" << std::endl;

    // Reconsider the decision if matching failed.

    // Run the topmost error hook if any.
    if (!m_current_state.error_hooks.empty()) {
        const auto& hook = m_current_state.error_hooks.top();
        std::cout << " -> run error hook " << (void*)hook.fn << std::endl;
        hook(*this);
        // No need to pop the hook, as `<DecisionType>::reconsider()` will take care of it.
    }

    // If we can reconsider a decision we have made earlier, do so.
    // We might need to go arbitrarily deep into the decision stack, since some decisions
    // we have made may have no more options remaining.
    while (true) {
        std::cout << " -> enter reconsider loop" << std::endl;
        if (m_decision_snapshots.empty()) {
            // TODO: signal about failed matching.
            std::terminate();
        }

        auto last_decision_snapshot = std::move(m_decision_snapshots.back());
        m_decision_snapshots.pop_back();
        std::cout << " ---> pop decision snapshot" << std::endl;
        auto has_reconsidered = last_decision_snapshot.decision.visit(
            [this, snapshot = std::move(last_decision_snapshot.snapshot)](auto& decision) {
                std::cout << " ---> reconsider decision" << std::endl;
                auto new_decision = decision.reconsider(*this, std::move(snapshot));
                if (!new_decision.has_value()) {
                    std::cout << " ---> failed" << std::endl;
                    // Options exhausted for this decision, try the next one.
                    return false;
                }
                std::cout << " ---> succeeded" << std::endl;
                decision = std::move(new_decision.value());
                return true;
            });

        if (has_reconsidered) {
            m_current_state.instructions.top().visit(
                [decision = std::move(last_decision_snapshot.decision)](
                    instruction::Execute& instruction) mutable {
                    instruction.forced_decision = std::move(decision);
                },
                [](auto&) {
                    throw std::logic_error("No snapshot could have been made just before am "
                                           "instruction other that `Execute`");
                });
            break;
        }
    }
    std::cout << " -> exit reconsider loop" << std::endl;
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
