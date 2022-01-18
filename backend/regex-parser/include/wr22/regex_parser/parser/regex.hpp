#pragma once

// wr22
#include <wr22/regex_parser/regex/part.hpp>

// stl
#include <string_view>

namespace wr22::regex_parser::parser {

regex::Part parse_regex(const std::string_view& regex);

}  // namespace wr22::regex_parser::parser
