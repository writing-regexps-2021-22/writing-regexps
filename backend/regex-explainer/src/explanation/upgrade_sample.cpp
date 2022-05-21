#include "wr22/regex_explainer/explanation/upgrade_sample.hpp"

namespace wr22::regex_explainer::explanation {

std::string upgrade_sample(const std::string& symbol, const std::string& sample) {
    std::string result = symbol + sample;
    return result;
}

}  // namespace wr22::regex_explainer::explanation
