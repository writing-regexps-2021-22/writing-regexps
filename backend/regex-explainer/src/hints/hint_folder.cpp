#include <wr22/regex_explainer/hints/hint_folder.hpp>

namespace wr22::regex_explainer::explanation {

Hint::Hint() : hint(""), additional_info("") {}

Hint::Hint(std::string hint_, std::string additional_info_) {
    hint = std::move(hint_);
    additional_info = std::move(additional_info_);
}

std::string Hint::get_hint() const {
    return hint;
}

std::string Hint::get_additional_information() const {
    return additional_info;
}

void to_json(nlohmann::json& j, const Hint& hint) {
    j = nlohmann::json::object();
    j["main_sentence"] = hint.main_sentence;
    j["hint"] = hint.get_hint();
    j["additional_info"] = hint.get_additional_information();
}

}  // namespace wr22::regex_explainer::explanation
