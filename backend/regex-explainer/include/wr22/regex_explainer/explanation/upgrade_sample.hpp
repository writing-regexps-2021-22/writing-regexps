#pragma once

#include "wr22/regex_explainer/explanation/sample_folder.hpp"

// STL
#include <string_view>

namespace wr22::regex_explainer::explanation {

/// Upgrades the default sample and makes it more specific
///
/// @param symbol the part to be added to the default sample.
/// @param sample initial default sample to be upgraded.
/// @return the upgraded sample in string type
std::string upgrade_sample(const std::string& symbol, const std::string& sample);

}  // namespace wr22::regex_explainer::explanation
