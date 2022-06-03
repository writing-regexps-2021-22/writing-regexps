// wr22
#include <wr22/regex_executor/algorithms/backtracking/executor.hpp>
#include <wr22/regex_executor/algorithms/backtracking/interpreter.hpp>
#include <wr22/regex_executor/algorithms/backtracking/match_failure.hpp>

namespace wr22::regex_executor::algorithms::backtracking {

Executor::Executor(const Regex& regex_ref) : m_regex_ref(regex_ref) {}

const Regex& Executor::regex_ref() const {
    return m_regex_ref.get();
}

MatchResult Executor::execute(const std::u32string_view& string) const {
    auto interpreter = Interpreter(regex_ref(), string);

    try {
        while (!interpreter.finished()) {
            interpreter.run_instruction();
        }
        interpreter.finalize();
    } catch (const MatchFailure&) {
        interpreter.finalize_error();
        return MatchResult{
            .matched = false,
            .steps = std::move(interpreter).into_steps(),
        };
    }

    return MatchResult{
        .matched = true,
        .captures = std::move(interpreter.current_state().captures),
        .steps = std::move(interpreter).into_steps(),
    };
}

}  // namespace wr22::regex_executor::algorithms::backtracking
