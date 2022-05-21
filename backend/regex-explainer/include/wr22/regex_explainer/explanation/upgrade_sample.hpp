#pragma once

#include "wr22/regex_explainer/explanation/sample_folder.hpp"

// STL
#include <string_view>

namespace wr22::regex_explainer::explanation {

std::string upgrade_sample(const std::string& symbol, const std::string_view& sample);

}  // namespace wr22::regex_explainer::explanation
