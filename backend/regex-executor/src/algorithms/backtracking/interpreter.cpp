// wr22
#include <wr22/regex_executor/algorithms/backtracking/decision_applicator.hpp>
#include <wr22/regex_executor/algorithms/backtracking/instruction.hpp>
#include <wr22/regex_executor/algorithms/backtracking/interpreter.hpp>
#include <wr22/regex_executor/algorithms/backtracking/part_executor.hpp>
#include <wr22/regex_executor/algorithms/backtracking/step.hpp>
#include <wr22/regex_executor/utils/spanned_ref.hpp>
#include <wr22/regex_parser/regex/part.hpp>
#include <wr22/unicode/conversion.hpp>

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

DecisionRef Interpreter::add_decision(Decision decision) {
    auto index = m_decision_snapshots.size();
    auto snapshot = InterpreterStateSnapshot{
        .state = m_current_state,
        .current_instruction = m_current_instruction.value(),
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

std::ostream& operator<<(std::ostream& out, const Instruction& instruction) {
    instruction.visit(
        [&out](const instruction::AddStep& instruction) {
            out << "add step: " << nlohmann::json(instruction.step).dump();
        },
        [&out](const instruction::Execute& instruction) {
            out << "execute part: " << instruction.part.span();
            if (instruction.forced_decision.has_value()) {
                out << " [forced decision]";
            }
        },
        [&out](const instruction::Run& instruction) {
            out << "run function: " << (void*)instruction.fn;
        });
    return out;
}

void Interpreter::run_instruction() {
    std::cout << "==> Run instruction [at " << cursor()
              << "]: " << wr22::unicode::to_utf8(m_string_ref.substr(0, cursor())) << "|"
              << wr22::unicode::to_utf8(m_string_ref.substr(cursor())) << std::endl;
    std::cout << " ### Counters: ";
    for (auto x : m_current_state.counters) {
        std::cout << x << ", ";
    }
    std::cout << ">>" << std::endl;
    std::cout << " ### Instructions:" << std::endl;
    for (auto x : m_current_state.instructions) {
        std::cout << "        <*> " << x << std::endl;
    }

    auto instruction = std::move(m_current_state.instructions.back());
    m_current_state.instructions.pop_back();
    m_current_instruction = instruction;
    std::cout << " -> " << instruction << std::endl;
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
                        auto executor = ApplicatorType::construct_executor(specific_part, decision);
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
            throw std::runtime_error("String not matched");
        }

        auto last_decision_snapshot = std::move(m_decision_snapshots.back());
        m_decision_snapshots.pop_back();
        std::cout << " ---> pop decision snapshot" << std::endl;
        auto current_instruction = last_decision_snapshot.snapshot.current_instruction;
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
            auto new_instruction = current_instruction.visit(
                [decision = std::move(last_decision_snapshot.decision)](
                    instruction::Execute instruction) mutable -> Instruction {
                    instruction.forced_decision = std::move(decision);
                    return instruction;
                },
                [](auto&) -> Instruction {
                    throw std::logic_error("No snapshot could have been made just before an "
                                           "instruction other that `Execute`");
                });
            m_current_state.instructions.push_back(std::move(new_instruction));
            break;
        }
    }
    std::cout << " -> exit reconsider loop" << std::endl;
}

void Interpreter::finalize() {
    add_step(step::End{
        .string_pos = cursor(),
        .result = step::End::Success{},
    });
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
