#pragma once

// wr22
#include <wr22/regex_executor/algorithms/backtracking/decision_ref.hpp>
#include <wr22/utils/adt.hpp>

namespace wr22::regex_executor::algorithms::backtracking {
struct Interpreter;

struct ErrorHook {
    using Context = wr22::utils::Adt<std::monostate, DecisionRef>;
    using Fn = void (*)(const Context& context, Interpreter& interpreter);
    Context ctx;
    Fn fn;

    void operator()(Interpreter& interpreter) const {
        fn(ctx, interpreter);
    }
};
}  // namespace wr22::regex_executor::algorithms::backtracking
