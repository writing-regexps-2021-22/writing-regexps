#pragma once

// wr22
#include <wr22/regex_executor/utils/spanned_ref.hpp>
#include <wr22/regex_parser/regex/part.hpp>

namespace wr22::regex_executor::algorithms::backtracking {

struct Interpreter;

class PartExecutor {
public:
    explicit PartExecutor(utils::SpannedRef<regex_parser::regex::Part> part_ref);
    bool execute(Interpreter& interpreter) const;

private:
    utils::SpannedRef<regex_parser::regex::Part> m_part_ref;
};

}  // namespace wr22::regex_executor::algorithms::backtracking
