// catch2
#include <catch2/catch.hpp>

// wr22
#include <wr22/regex_parser/parser/regex.hpp>
#include <wr22/regex_parser/regex/part.hpp>
#include <wr22/regex_parser/utils/utf8_string_view.hpp>

using wr22::regex_parser::parser::parse_regex;
using wr22::regex_parser::regex::Part;
using wr22::regex_parser::utils::UnicodeStringView;
namespace part = wr22::regex_parser::regex::part;

TEST_CASE("Basics", "[regex]") {
    REQUIRE(
        parse_regex(UnicodeStringView("a|bc"))
        == Part(part::Alternatives{std::vector<Part>{
            part::Literal{'a'},
            part::Sequence{std::vector<Part>{part::Literal{'b'}, part::Literal{'c'}}}}}));
}
