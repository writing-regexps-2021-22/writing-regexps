#pragma once

// wr22
#include "wr22/regex_explainer/explanation/explanation_folder.hpp"
#include "wr22/utils/adt.hpp"

// STL
#include <nlohmann/json_fwd.hpp>
#include <string>

// variant
#include <variant>

// nlohmann
#include <nlohmann/json.hpp>

namespace wr22::regex_explainer::explanation {

typedef wr22::utils::Adt<std::string, std::u32string, uint32_t> type_of_sentence;

class Explanation {
public:
    Explanation();
    Explanation(type_of_sentence explanation_, size_t depth_, bool bold_ = false);

    type_of_sentence get_explanation() const;
    [[nodiscard]] size_t get_depth() const;
    [[nodiscard]] bool is_bold() const;

private:
    type_of_sentence explanation;
    size_t depth;
    bool bold;
};

void to_json(nlohmann::json& j, const Explanation& explanation);

}  // namespace wr22::regex_explainer::explanation
