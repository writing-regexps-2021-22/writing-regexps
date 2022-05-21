#pragma once

// wr22
#include "wr22/regex_explainer/explanation/explanation_folder.hpp"
#include "wr22/regex_parser/regex/part.hpp"
#include "wr22/regex_parser/span/span.hpp"

// STL
#include <string_view>

namespace wr22::regex_explainer::explanation {

    using regex_parser::regex::part::Empty;
    using regex_parser::regex::part::Literal;
    using regex_parser::regex::part::Alternatives;
    using regex_parser::regex::part::Sequence;
    using regex_parser::regex::part::Group;
    using regex_parser::regex::part::Optional;
    using regex_parser::regex::part::Plus;
    using regex_parser::regex::part::Star;
    using regex_parser::regex::part::Wildcard;
    using regex_parser::regex::part::CharacterClass;

    /// This is functions that returns the sample for the node from AST as the std::string_view.
    /// The function is overloaded and returns a different pattern of type "std::string_view"
    /// depending on the type of the argument it takes.
    ///
    /// The function for type "Empty" is always default because it does not have any meaningful information.
    std::string get_sample(const Empty &vertex);

    /// The function describes which literal matched and what index it has in the ASCII table.
    /// The function is case sensitive.
    std::vector<std::string> get_sample(const Literal &vertex);

    /// The function lists all alternatives that needed to be mached and describes each one in detail.
    std::string get_sample(const Alternatives &vertex);

    /// The function lists all items of the sequence list and match there explanation one after another.
    /// There is no information about the type of this vertex.
    std::string get_sample(const Sequence &vertex);

    /// he function says that it is group and describes it in detail.
    std::string get_sample(const Group &vertex);

    /// Next three functions return the explanation of some quantifier.
    /// They are always the same and do not require any modifications depending on the situation.
    std::string get_sample(const Optional &vertex);

    std::string get_sample(const Plus &vertex);

    std::string get_sample(const Star &vertex);

    /// The function returns the explanation for some regex basic that is always the same
    /// and does not require any modifications depending on the situation.
    std::string get_sample(const Wildcard &vertex);

    /// The function lists the symbols from the certain set that need to be matched.
    /// The function is case sensitive.
    std::vector<std::string> get_sample(const CharacterClass &vertex);

}  // namespace wr22::regex_explainer::explanation
