#pragma once

// wr22
#include "wr22/regex_executor/algorithms/backtracking/match_result.hpp"
#include <string_view>
#include <wr22/regex_executor/regex.hpp>
#include <wr22/regex_executor/algorithms/backtracking/executor.hpp>

// stl
#include <functional>

namespace wr22::regex_executor {

class Executor {
public:
    using BacktrackingResult = algorithms::backtracking::MatchResult;

    explicit Executor(const Regex& regex_ref);

    const Regex& regex_ref() const;
    BacktrackingResult execute(const std::u32string_view& string);

private:
    using BacktrackingExecutor = algorithms::backtracking::Executor;

    BacktrackingExecutor m_executor;
};

}
