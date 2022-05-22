// wr22
#include "wr22/regex_explainer/explanation/sample_folder.hpp"

// STL
#include <string_view>

namespace wr22::regex_explainer::explanation {

    /// The function returns a sample for the AST node depending on its type.
    ///
    /// If the sample consists of one part, then simply returns std::string.
    /// If the sample consists of several parts, then a vector of patterns with type std::string is returned,
    /// which should then make up a whole sentence / paragraph.

    std::string get_sample(const Empty &vertex) {
        std::string pattern1 = "Is empty";
        return pattern1;
    }

    std::vector<std::string> get_sample(const Literal &vertex) {
        std::string pattern1 = "matches the character ",
                pattern2 = "with index",
                pattern3 = "literally (case sensitive)";
        return {pattern1, pattern2, pattern3};
    }

    std::string get_sample(const Alternatives &vertex) {
        std::string pattern1 = " Alternative";
        return pattern1;
    }

    std::string get_sample(const Sequence &vertex) {
        std::string pattern1 = "\'";
        return pattern1;
    }

    /// TO DO
    std::vector<std::string> get_sample(const Group &vertex) {
        std::string pattern1 = "Capturing Group",
                    pattern2 = "Non-Capturing Group",
                    pattern3 = "Named Capture Group ";
        return {pattern1, pattern2, pattern3};
    }

    std::string get_sample(const Optional &vertex) {
        std::string pattern1 = " matches the previous token between zero and one times, "
                                    "as many times as possible, giving back as needed (greedy)";
        return pattern1;
    }

    std::string get_sample(const Plus &vertex) {
        std::string pattern1 = " matches the previous token between one and unlimited times, "
                                    "as many times as possible, giving back as needed (greedy)";
        return pattern1;
    }

    std::string get_sample(const Star &vertex) {
        std::string pattern1 = " matches the previous token between zero and unlimited times, "
                                    "as many times as possible, giving back as needed (greedy)";
        return pattern1;
    }

    std::string get_sample(const Wildcard &vertex) {
        std::string pattern1 = " matches any character (except for line terminators)";
        return pattern1;
    }

    std::vector<std::string> get_sample(const CharacterClass &vertex) {
        std::string main_pattern = "Match a single character present in the list.",
                pattern1 = "matches a single character in the range between",
                pattern2 = "and",
                pattern3 = "(case sensitive)";
        return {main_pattern, pattern1, pattern2, pattern3};
    }

}  // namespace wr22::regex_explainer::explanation
