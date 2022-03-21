#pragma once

// wr22
#include <wr22/regex_parser/regex/part.hpp>

// stl
#include <string_view>

namespace wr22::regex_parser::parser {

/// Parse a regular expression into its AST.
///
/// The regular expression is a string view in the UTF-32 encoding. It is parsed and its object
/// representation (see the docs for `regex::SpannedPart`) is built. The returned representation is
/// an owned object and its lifetime does not depend on the lifetime of the `regex` argument.
///
/// If the parsing fails, an exception is thrown. `errors::ParseError` is the base class for all
/// exceptions thrown from this function, but more specific exceptions may be caught and handled
/// separately. See the docs for the `errors.hpp` file for details.
///
/// @returns the parsed regex AST if the parsing succeeds.
/// @throws errors::ParseError if the parsing fails.
regex::SpannedPart parse_regex(const std::u32string_view& regex);

}  // namespace wr22::regex_parser::parser
