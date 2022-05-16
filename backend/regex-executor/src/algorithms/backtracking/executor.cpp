// wr22
#include <wr22/regex_executor/algorithms/backtracking/executor.hpp>
#include <wr22/regex_executor/algorithms/backtracking/interpreter.hpp>

namespace wr22::regex_executor::algorithms::backtracking {

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
