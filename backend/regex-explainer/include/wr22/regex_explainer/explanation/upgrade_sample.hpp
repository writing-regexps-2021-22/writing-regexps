#pragma once

#include "wr22/regex_explainer/explanation/sample_folder.hpp"

// STL
#include <string_view>

namespace wr22::regex_explainer::explanation {

    const size_t alternative_sz = 13;
    const size_t optional_sz = 114;
    const size_t plus_sz = 119;
    const size_t star_sz = 120;
    const size_t wildcard_sz = 52;

    std::string_view upgrade_sample_altertative(char symbol, const std::string_view &sample);

    std::string_view upgrade_sample_optional(char symbol, const std::string_view &sample);

    std::string_view upgrade_sample_plus(char symbol, const std::string_view &sample);

    std::string_view upgrade_sample_star(char symbol, const std::string_view &sample);

    std::string_view upgrade_sample_wildcard(char symbol, const std::string_view &sample);

} // wr22::regex_explainer::explanation
