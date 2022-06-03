// wr22
#include <wr22/regex_executor/executor.hpp>

namespace wr22::regex_executor {

Executor::Executor(const Regex& regex_ref) : m_executor(regex_ref) {}

const Regex& Executor::regex_ref() const {
    return m_executor.regex_ref();
}

Executor::BacktrackingResult Executor::execute(const std::u32string_view& string) {
    return m_executor.execute(string);
}

}  // namespace wr22::regex_executor
