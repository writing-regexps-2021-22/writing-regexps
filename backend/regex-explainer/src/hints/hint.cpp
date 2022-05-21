// wr22
#include "wr22/regex_explainer/hints/hint.hpp"

// string
#include <string>

namespace wr22::regex_explainer::hints {

    Hint get_hint(const ParseError &error) {
        return Hint{};
    }

    Hint get_hint(const UnexpectedEnd &error) {
        std::string hint = "Unexpected end of input at position "
                           + std::to_string(error.position())
                           + ": expected "
                           + error.expected();

        std::string additional_info;
        if (error.expected() == ")") {
            additional_info = "Incomplete group structure";
        } else if (error.expected() == "]") {
            additional_info = "Character class missing closing bracket";
        }

        return Hint{hint, additional_info};
    }

    Hint get_hint(const ExpectedEnd &error) {
        std::string hint = "Expected the input to end at position "
                           + std::to_string(error.position())
                           + ", but got the character "
                           + wr22::unicode::to_utf8(error.char_got());

        return Hint{hint};
    }

    Hint get_hint(const UnexpectedChar &error) {
        std::string hint = "Expected "
                           + error.expected()
                           + ", but got the character "
                           + wr22::unicode::to_utf8(error.char_got())
                           + " at position "
                           + std::to_string(error.position());

        std::string additional_info;

        if (error.char_got() == U'?' || error.char_got() == U'*' || error.char_got() == U'+') {
            additional_info = wr22::unicode::to_utf8(error.char_got())
                              + " The preceding token is not quantifiable";
        }

        return Hint{hint, additional_info};
    }

    Hint get_hint(const InvalidRange &error) {
        std::string hint = "Invalid character range "
                                 + wr22::unicode::to_utf8(error.first())
                                 + "–"
                                 + wr22::unicode::to_utf8(error.last())
                                 + " at positions from "
                                 + std::to_string(error.span().begin())
                                 + " to "
                                 + std::to_string(error.span().end());

        std::string additional_info = "– Character range is out of order";

        return Hint{hint, additional_info};
    }

}  // namespace wr22::regex_explainer::hints



