#pragma once

// wr22
#include "wr22/regex_explainer/hints/hint_folder.hpp"
#include "wr22/regex_parser/parser/errors.hpp"
#include "wr22/unicode/conversion.hpp"

// vector
#include <vector>

namespace wr22::regex_explainer::hints {

    using explanation::Hint;
    using regex_parser::parser::errors::ParseError;
    using regex_parser::parser::errors::UnexpectedEnd;
    using regex_parser::parser::errors::ExpectedEnd;
    using regex_parser::parser::errors::UnexpectedChar;
    using regex_parser::parser::errors::InvalidRange;

    /// If any exception was thrown, then there is an error in the regular expression. If the error is related
    /// to syntax, then this function returns a hint on how to fix it.
    /// In case of other errors, the function returns information about where the failure occurred and
    /// some additional information in some cases.

    Hint get_hint(const UnexpectedEnd &error);

    Hint get_hint(const ExpectedEnd &error);

    Hint get_hint(const UnexpectedChar &error);

    Hint get_hint(const InvalidRange &error);

}  // namespace wr22::regex_explainer::hints



