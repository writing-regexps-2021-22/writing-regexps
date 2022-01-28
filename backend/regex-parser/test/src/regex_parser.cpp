// catch2
#include <catch2/catch.hpp>

// regex parser tests
#include <regex_parser_tests/equals_matcher.hpp>
#include <regex_parser_tests/vec.hpp>

// wr22
#include <wr22/regex_parser/parser/errors.hpp>
#include <wr22/regex_parser/parser/regex.hpp>
#include <wr22/regex_parser/regex/capture.hpp>
#include <wr22/regex_parser/regex/part.hpp>
#include <wr22/regex_parser/utils/utf8_string_view.hpp>

// STL
#include <string_view>

using Catch::Predicate;
using regex_parser_tests::Equals;
using regex_parser_tests::vec;
using wr22::regex_parser::parser::parse_regex;
using wr22::regex_parser::parser::errors::ExpectedEnd;
using wr22::regex_parser::parser::errors::UnexpectedEnd;
using wr22::regex_parser::regex::Capture;
using wr22::regex_parser::regex::Part;
using wr22::regex_parser::utils::UnicodeStringView;
namespace part = wr22::regex_parser::regex::part;
namespace capture = wr22::regex_parser::regex::capture;

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
    CHECK(parse_regex(UnicodeStringView("foo")) == Part(lit(U"foo")));
    CHECK(parse_regex(UnicodeStringView("x")) == Part(part::Literal{U'x'}));
    CHECK(parse_regex(UnicodeStringView("")) == Part(part::Empty{}));
    CHECK(parse_regex(UnicodeStringView("тест юникода")) == Part(lit(U"тест юникода")));
    CHECK(parse_regex(UnicodeStringView("\t  whitespace   ")) == Part(lit(U"\t  whitespace   ")));
}

TEST_CASE("Alternatives", "[regex]") {
    CHECK(
        parse_regex(UnicodeStringView("foo|bar"))
        == Part(part::Alternatives{vec<Part>(lit(U"foo"), lit(U"bar"))}));
    CHECK(
        parse_regex(UnicodeStringView("foo|x"))
        == Part(part::Alternatives{vec<Part>(lit(U"foo"), part::Literal{U'x'})}));
    CHECK(
        parse_regex(UnicodeStringView("a|b|c|test|d|e"))
        == Part(part::Alternatives{vec<Part>(
            part::Literal{U'a'},
            part::Literal{U'b'},
            part::Literal{U'c'},
            lit(U"test"),
            part::Literal{U'd'},
            part::Literal{U'e'})}));
    CHECK(
        parse_regex(UnicodeStringView("a||b"))
        == Part(part::Alternatives{
            vec<Part>(part::Literal{U'a'}, part::Empty{}, part::Literal{U'b'})}));
    CHECK(
        parse_regex(UnicodeStringView("a|b|"))
        == Part(part::Alternatives{
            vec<Part>(part::Literal{U'a'}, part::Literal{U'b'}, part::Empty{})}));
    CHECK(
        parse_regex(UnicodeStringView("|a|b"))
        == Part(part::Alternatives{
            vec<Part>(part::Empty{}, part::Literal{U'a'}, part::Literal{U'b'})}));
    CHECK(
        parse_regex(UnicodeStringView("|"))
        == Part(part::Alternatives{vec<Part>(part::Empty{}, part::Empty{})}));
    CHECK(
        parse_regex(UnicodeStringView("|||"))
        == Part(part::Alternatives{
            vec<Part>(part::Empty{}, part::Empty{}, part::Empty{}, part::Empty{})}));
}

TEST_CASE("Groups", "[regex]") {
    CHECK(
        parse_regex(UnicodeStringView("(aaa)"))
        == Part(part::Group(capture::Index{}, lit(U"aaa"))));
    CHECK(
        parse_regex(UnicodeStringView("(?:foobar)"))
        == Part(part::Group(capture::None{}, lit(U"foobar"))));
    CHECK(
        parse_regex(UnicodeStringView("(?<x>foobar)"))
        == Part(part::Group(capture::Name{"x"}, lit(U"foobar"))));
    CHECK(
        parse_regex(UnicodeStringView("(?<quux>12345)"))
        == Part(part::Group(capture::Name{"quux"}, lit(U"12345"))));
    CHECK(
        parse_regex(UnicodeStringView("(?'abc123'xyz)"))
        == Part(part::Group(capture::Name{"abc123"}, lit(U"xyz"))));
    CHECK(
        parse_regex(UnicodeStringView("(?P<name>group)"))
        == Part(part::Group(capture::Name{"name"}, lit(U"group"))));
    CHECK(
        parse_regex(UnicodeStringView("(?P<тест>юникода)"))
        == Part(part::Group(capture::Name{"тест"}, lit(U"юникода"))));
    CHECK(
        parse_regex(UnicodeStringView("()")) == Part(part::Group(capture::Index{}, part::Empty{})));
    CHECK(
        parse_regex(UnicodeStringView("(?:)"))
        == Part(part::Group(capture::None{}, part::Empty{})));
    CHECK(
        parse_regex(UnicodeStringView("(?'bb')"))
        == Part(part::Group(capture::Name{"bb"}, part::Empty{})));
    CHECK(
        parse_regex(UnicodeStringView("(?P<ccc>)"))
        == Part(part::Group(capture::Name{"ccc"}, part::Empty{})));
    CHECK_THROWS_MATCHES("(a", UnexpectedEnd, Predicate<UnexpectedEnd>([](const auto& e) {
                             return e.position() == 2;
                         }));
}
