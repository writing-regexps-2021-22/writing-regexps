#pragma once

// wr22
#include <wr22/regex_executor/regex.hpp>
#include <wr22/regex_executor/algorithms/backtracking/executor.hpp>

// stl
#include <functional>

namespace wr22::regex_executor {

class Executor {
public:
    explicit Executor(const Regex& regex_ref);

    const Regex& regex_ref() const;

private:
    using BacktrackingExecutor = algorithms::backtracking::Executor;

    BacktrackingExecutor m_executor;
};

}
