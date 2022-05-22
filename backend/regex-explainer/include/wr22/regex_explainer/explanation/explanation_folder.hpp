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

/// A class for holding an explanation tree element.
class Explanation {
public:
    Explanation();

    /// Constructor.
    ///
    /// @param explanation_ is the variant type that represents some part of explanation.
    /// @param depth_ is the depth of the current explanation in the resulting tree of explanations.
    /// @param bold is boolean variable indicating the need to highlight the color in bold.
    Explanation(type_of_sentence explanation_, size_t depth_, bool bold_ = false);

    /// @return the explanation for current spanned_part
    [[nodiscard]] type_of_sentence get_explanation() const;
    /// @return the depth of current spanned_part
    [[nodiscard]] size_t get_depth() const;
    /// @return the bold type indicator
    [[nodiscard]] bool is_bold() const;

private:
    type_of_sentence explanation;
    size_t depth;
    bool bold;
};

void to_json(nlohmann::json& j, const Explanation& explanation);

}  // namespace wr22::regex_explainer::explanation
