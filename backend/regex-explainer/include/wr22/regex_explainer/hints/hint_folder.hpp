#pragma once

// STL
#include <string>
#include <string_view>
#include <utility>

// nlohmann
#include <nlohmann/json.hpp>

namespace wr22::regex_explainer::explanation {

class Hint {
public:
    Hint();
    Hint(std::string hint_, std::string additional_info_ = "");

    std::string get_hint() const;
    std::string get_additional_information() const;

    static constexpr const char* main_sentence =
        "Your pattern contains one or more errors, please see the explanation section above";

private:
    std::string hint;
    std::string additional_info;
};

void to_json(nlohmann::json& j, const Hint& hint);

}  // namespace wr22::regex_explainer::explanation
