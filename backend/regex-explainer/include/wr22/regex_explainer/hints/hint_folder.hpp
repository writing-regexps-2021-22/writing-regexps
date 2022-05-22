#pragma once

// STL
#include <string>
#include <string_view>
#include <utility>

// nlohmann
#include <nlohmann/json.hpp>

namespace wr22::regex_explainer::explanation {

/// A class for holding a hint.
class Hint {
public:
    Hint();

    /// Constructor.
    ///
    /// @param hint_ is a string containing information about the location where the error occurred
    /// and the expected character.
    /// @param additional_info_ if an optional variable containing additional information about
    /// the error.
    explicit Hint(std::string hint_, std::string additional_info_ = "");

    /// @return the hint for current error
    [[nodiscard]] std::string get_hint() const;
    /// @return the additional information for current error
    [[nodiscard]] std::string get_additional_information() const;

    /// This is a sentence for any error indicating that there is an error in the regular expression.
    static constexpr const char* main_sentence =
        "Your pattern contains one or more errors, please see the explanation section above";

private:
    std::string hint;
    std::string additional_info;
};

void to_json(nlohmann::json& j, const Hint& hint);

}  // namespace wr22::regex_explainer::explanation
