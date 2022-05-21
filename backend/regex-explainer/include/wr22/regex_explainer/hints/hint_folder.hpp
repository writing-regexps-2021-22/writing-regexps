#pragma once

// STL
#include <string>
#include <string_view>
#include <utility>

namespace wr22::regex_explainer::explanation {

    class Hint {
    public:
        Hint() : hint(""), additional_info("") {}

        Hint(std::string hint_, std::string additional_info_ = "") {
            hint = std::move(hint_);
            additional_info = std::move(additional_info_);
        }

        std::string get_hint() {
            return hint;
        }

        std::string get_additional_information() {
            return additional_info;
        }

    private:
        std::string main_sentence = "Your pattern contains one or more errors, please see the explanation section above";
        std::string hint;
        std::string additional_info;
    };

} // wr22::regex_explainer::explanation
