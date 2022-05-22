#pragma once

// wr22
#include "wr22/regex_parser/regex/part.hpp"
#include "wr22/regex_parser/span/span.hpp"
#include "wr22/regex_explainer/explanation/explanation_folder.hpp"
#include "wr22/regex_explainer/explanation/sample_folder.hpp"
#include "wr22/regex_explainer/explanation/upgrade_sample.hpp"
#include "wr22/unicode/conversion.hpp"

// STL
#include <string_view>

namespace wr22::regex_explainer::explanation {

    using regex_parser::regex::SpannedPart;

    /// The function returns the vector processed information about each node of the syntax tree.
    /// Thus, a complete description of the work of the written regular expression is created.
    ///
    /// @param spanned_part is the current node of syntax tree to be explained.
    /// @param depth is the depth of the current explanation in the resulting tree of explanations.
    ///
    /// @return a vector of elements that have type of class Explanation.
    std::vector<Explanation>
    get_full_explanation(const SpannedPart &spanned_part, size_t depth = 0);

}  // namespace wr22::regex_explainer::explanation
