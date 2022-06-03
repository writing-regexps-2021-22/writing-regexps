#include <wr22/regex_executor/algorithms/backtracking/instruction.hpp>
#include <wr22/regex_executor/algorithms/backtracking/interpreter.hpp>

namespace wr22::regex_executor::algorithms::backtracking {

void instruction::Run::operator()(Interpreter& interpreter) const {
    fn(ctx, interpreter);
}

}  // namespace wr22::regex_executor::algorithms::backtracking
