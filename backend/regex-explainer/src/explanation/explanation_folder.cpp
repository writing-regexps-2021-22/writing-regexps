// wr22
#include <wr22/regex_explainer/explanation/explanation_folder.hpp>

namespace wr22::regex_explainer::explanation {

Explanation::Explanation() : explanation(""), depth(0), bold(false) {}

Explanation::Explanation(type_of_sentence explanation_, size_t depth_, bool bold_)
    : explanation(explanation_), depth(depth_), bold(bold_) {}

type_of_sentence Explanation::get_explanation() const {
    return explanation;
}

size_t Explanation::get_depth() const {
    return depth;
}

bool Explanation::is_bold() const {
    return bold;
}

void to_json(nlohmann::json& j, const Explanation& explanation) {
    j = nlohmann::json::object();
    j["depth"] = explanation.get_depth();
    j["bold"] = explanation.is_bold();
    auto& json_explanation = j["explanation"];
    explanation.get_explanation().visit(
        [&json_explanation](const std::string_view& sv) { json_explanation = sv; },
        [&json_explanation](const std::u32string_view& sv) { json_explanation = sv; },
        [&json_explanation](uint32_t num) { json_explanation = num; });
}

}  // namespace wr22::regex_explainer::explanation
