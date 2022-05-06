#pragma once

// wr22
#include <wr22/regex_executor/algorithms/backtracking/match_result.hpp>
#include <wr22/regex_executor/regex.hpp>

// stl
#include <functional>
#include <string_view>

namespace wr22::regex_executor::algorithms::backtracking {

class Executor {
public:
    explicit Executor(const Regex& regex_ref);

    const Regex& regex_ref() const;
    MatchResult execute(const std::u32string_view& string) const;

private:
    std::reference_wrapper<const Regex> m_regex_ref;
};

}  // namespace wr22::regex_executor::algorithms::backtracking
