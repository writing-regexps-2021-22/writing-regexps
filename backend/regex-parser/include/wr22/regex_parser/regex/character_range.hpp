#pragma once

// stl
#include <stdexcept>

// fmt
#include <fmt/core.h>

// nlohmann
#include <nlohmann/json.hpp>

namespace wr22::regex_parser::regex {

struct InvalidCharacterRange : public std::runtime_error {
    explicit InvalidCharacterRange(char32_t first, char32_t last);
};

/// A non-empty character range, possibly containing only one character.
class CharacterRange {
public:
    /// Construct a character range given its two inclusive endpoints.
    ///
    /// @throws `InvalidCharacterRange` if `last < first`.
    static CharacterRange from_endpoints(char32_t first, char32_t last);

    /// Construct a character range containing one given character.
    static CharacterRange from_single_character(char32_t character) noexcept;

    /// Get the left range bound (inclusive).
    char32_t first() const noexcept;

    /// Get the right range bound (inclusive).
    char32_t last() const noexcept;

    /// Check if this range contains a given character.
    bool contains(char32_t character) const noexcept;

    /// Check if this range contains exactly one character.
    bool is_single_character() const noexcept;

    bool operator==(const CharacterRange& rhs) const noexcept = default;

private:
    /// Constructor. Use `static` methods for construction.
    ///
    /// @throws `InvalidCharacterRange` if `last < first`.
    explicit CharacterRange(char32_t first, char32_t last);

    char32_t m_first;
    char32_t m_last;
};

void to_json(nlohmann::json& j, const CharacterRange& range);

}  // namespace wr22::regex_parser::regex
