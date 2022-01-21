#pragma once

// wr22
#include <wr22/regex_parser/regex/part.hpp>
#include <wr22/regex_parser/utils/utf8_string_view.hpp>

// stl
#include <string_view>

namespace wr22::regex_parser::parser {

regex::Part parse_regex(const utils::UnicodeStringView& regex);

}  // namespace wr22::regex_parser::parser
