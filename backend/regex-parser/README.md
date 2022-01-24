# regex-parser

This is the library for parsing regular expressions. A regular expression is represented as
a UTF-8 encoded string (the type `wr22::regex_parser::utils::UnicodeStringView` type). The main
function you might need is `wr22::regex_parser::parser::parse_regex`. This function parses this unicode
string into the regex abstract syntax tree, which is represented by the `wr22::regex_parser::regex::Part`
type. Please take a look at the API reference for details.
