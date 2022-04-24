#pragma once

// wr22
#include <wr22/regex_parser/regex/spanned_character_range.hpp>

// stl
#include <vector>

namespace wr22::regex_parser::regex {

/// A character class representation: a list of character ranges plus some additional properties.
struct CharacterClassData {
    /// List of character ranges and their spans.
    ///
    /// Example: for `[a-z123]` it is
    /// `['a'..'z' [1..4], '1'..'1' [4..4], '2'..'2' [5..5], '3'..'3' [6..6]]`
    /// (both ends in character ranges are included; [A..B] are spans with the left end included
    /// and the right one excluded).
    std::vector<SpannedCharacterRange> ranges;

    /// True if the match is inverted (i.e. `[^something]`), false otherwise.
    bool inverted;

    bool operator==(const CharacterClassData& rhs) const = default;
};

}
