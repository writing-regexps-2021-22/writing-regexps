#pragma once

// wr22
#include <wr22/regex_parser/regex/character_range.hpp>

// stl
#include <vector>

namespace wr22::regex_parser::regex {

/// A character class representation: a list of character ranges plus some additional properties.
struct CharacterClassData {
    /// List of character ranges.
    ///
    /// Example: for `[a-z123]` it is `['a'..'z', '1'..'1', '2'..'2' and '3'..'3']` (both ends in ranges
    /// are included).
    std::vector<CharacterRange> ranges;

    /// True if the match is inverted (i.e. `[^something]`), false otherwise.
    bool inverted;

    bool operator==(const CharacterClassData& rhs) const = default;
};

}
