#pragma once

// wr22
#include <wr22/regex_executor/utils/spanned_ref.hpp>
#include <wr22/regex_executor/algorithms/backtracking/step.hpp>
#include <wr22/regex_parser/span/span.hpp>
#include <wr22/regex_parser/regex/part.hpp>
#include <wr22/utils/adt.hpp>
#include <wr22/regex_executor/algorithms/backtracking/decision.hpp>

// stl
#include <optional>

namespace wr22::regex_executor::algorithms::backtracking {

struct Interpreter;

namespace instruction {
    struct Execute {
        utils::SpannedRef<regex_parser::regex::Part> part;
        std::optional<Decision> forced_decision;
    };

    struct AddStep {
        Step step;
    };

    struct Run {
        using Context = wr22::utils::Adt<
            std::monostate,
            utils::SpannedRef<regex_parser::regex::part::Star>,
            utils::SpannedRef<regex_parser::regex::part::Plus>,
            utils::SpannedRef<regex_parser::regex::part::Optional>>;
        using Fn = void (*)(const Context& ctx, Interpreter& interpreter);
        Context ctx;
        Fn fn;

        void operator()(Interpreter& interpreter) const;
    };

    using Adt = wr22::utils::Adt<Execute, AddStep, Run>;
};  // namespace instruction

using Instruction = instruction::Adt;
}  // namespace wr22::regex_executor::algorithms::backtracking
