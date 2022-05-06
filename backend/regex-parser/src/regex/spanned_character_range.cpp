// wr22
#include <wr22/regex_parser/regex/spanned_character_range.hpp>

// stl
#include <ostream>

// fmt
#include <fmt/core.h>
#include <fmt/ostream.h>

namespace wr22::regex_parser::regex {

std::ostream& operator<<(std::ostream& out, const SpannedCharacterRange& range) {
    fmt::print(out, "{} [{}]", range.range, range.span);
    return out;
}

void to_json(nlohmann::json& j, const SpannedCharacterRange& range) {
    j = nlohmann::json::object();
    j["range"] = range.range;
    j["span"] = range.span;
}

}
