// catch2
#include <catch2/catch.hpp>

// regex parser tests
#include <regex_parser_tests/vec.hpp>

// wr22
#include <wr22/regex_parser/parser/regex.hpp>
#include <wr22/regex_parser/regex/part.hpp>
#include <wr22/regex_parser/utils/utf8_string_view.hpp>

// STL
#include <string_view>

using regex_parser_tests::vec;
using wr22::regex_parser::parser::parse_regex;
using wr22::regex_parser::regex::Part;
using wr22::regex_parser::utils::UnicodeStringView;
namespace part = wr22::regex_parser::regex::part;

namespace {
Part lit(const std::u32string_view& str) {
    std::vector<Part> chars;
    chars.reserve(str.size());
    for (auto c : str) {
        chars.push_back(part::Literal{c});
    }
    return part::Sequence{std::move(chars)};
}
}  // namespace

TEST_CASE("Basics", "[regex]") {
    REQUIRE(parse_regex(UnicodeStringView("foo")) == Part(lit(U"foo")));
}

TEST_CASE("Alternatives", "[regex]") {
    REQUIRE(
        parse_regex(UnicodeStringView("a|bc"))
        == Part(part::Alternatives{vec<Part>(
            part::Literal{U'a'},
            part::Sequence{vec<Part>(part::Literal{U'b'}, part::Literal{U'c'})})}));
}
