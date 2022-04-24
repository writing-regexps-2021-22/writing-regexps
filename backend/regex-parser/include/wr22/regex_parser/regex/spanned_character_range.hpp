#pragma once

// wr22
#include <wr22/regex_parser/regex/character_range.hpp>
#include <wr22/regex_parser/span/span.hpp>

// stl
#include <iosfwd>

// nlohmann
#include <nlohmann/json.hpp>

namespace wr22::regex_parser::regex {

/// A `CharacterRange` with its span.
struct SpannedCharacterRange {
    CharacterRange range;
    span::Span span;

    bool operator==(const SpannedCharacterRange& rhs) const = default;
};

std::ostream& operator<<(std::ostream& out, const SpannedCharacterRange& range);
void to_json(nlohmann::json& j, const SpannedCharacterRange& range);

}  // namespace wr22::regex_parser::regex
