// wr22
#include <wr22/regex_executor/executor.hpp>

namespace wr22::regex_executor {

Executor::Executor(const Regex& regex_ref) : m_executor(regex_ref) {}

const Regex& Executor::regex_ref() const {
    return m_executor.regex_ref();
}

}  // namespace wr22::regex_executor
