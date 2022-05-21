#pragma once

// wr22
#include "wr22/regex_explainer/explanation/explanation_folder.hpp"

// STL
#include <string_view>

// variant
#include <variant>

namespace wr22::regex_explainer::explanation {

    typedef std::variant<std::string, std::u32string, uint32_t> type_of_sentence;

    class Explanation {
    public:
        Explanation() : explanation(""), depth(0), bold(false) {}

        Explanation(type_of_sentence explanation_, size_t depth_, bool bold_ = false) {
            explanation = explanation_;
            depth = depth_;
            bold = bold_;
        }

        template<class String_view>
        String_view get_explanation() {
            std::visit([](auto &&exp) {
                return exp;
            }, explanation);
        }

        [[nodiscard]] size_t get_depth() const {
            return depth;
        }

        [[nodiscard]] bool is_bold() const {
            return bold;
        }

    private:
        type_of_sentence explanation;
        size_t depth;
        bool bold;
    };

} // wr22::regex_explainer::explanation
