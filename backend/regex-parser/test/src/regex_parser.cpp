// catch2
#include <catch2/catch.hpp>

// regex parser tests
#include <regex_parser_tests/equals_matcher.hpp>
#include <regex_parser_tests/vec.hpp>

// wr22
#include <wr22/regex_parser/parser/errors.hpp>
#include <wr22/regex_parser/parser/regex.hpp>
#include <wr22/regex_parser/regex/capture.hpp>
#include <wr22/regex_parser/regex/named_capture_flavor.hpp>
#include <wr22/regex_parser/regex/part.hpp>

// STL
#include <string_view>

using Catch::Predicate;
using regex_parser_tests::vec;
using wr22::regex_parser::parser::parse_regex;
using wr22::regex_parser::parser::errors::ExpectedEnd;
using wr22::regex_parser::parser::errors::UnexpectedChar;
using wr22::regex_parser::parser::errors::UnexpectedEnd;
using wr22::regex_parser::regex::NamedCaptureFlavor;
using wr22::regex_parser::regex::SpannedPart;
using wr22::regex_parser::span::Span;
namespace part = wr22::regex_parser::regex::part;
namespace capture = wr22::regex_parser::regex::capture;

namespace {
SpannedPart lit(const std::u32string_view& str, size_t begin) {
    std::vector<SpannedPart> chars;
    chars.reserve(str.size());
    size_t offset = 0;
    for (auto c : str) {
        chars.push_back(SpannedPart(part::Literal(c), Span::make_single_position(begin + offset)));
        ++offset;
    }
    return SpannedPart(part::Sequence(std::move(chars)), Span::make_with_length(begin, offset));
}

SpannedPart lit_char(char32_t c, size_t position) {
    return SpannedPart(part::Literal(c), Span::make_single_position(position));
}

SpannedPart empty(size_t position) {
    return SpannedPart(part::Empty(), Span::make_empty(position));
}

Span whole(size_t length) {
    return Span::make_with_length(0, length);
}
}  // namespace

TEST_CASE("Basics", "[regex]") {
    CHECK(parse_regex(U"foo") == lit(U"foo", 0));
    CHECK(parse_regex(U"x") == lit_char(U'x', 0));
    CHECK(parse_regex(U"") == empty(0));
    CHECK(parse_regex(U"тест юникода") == lit(U"тест юникода", 0));
    CHECK(parse_regex(U"\t  whitespace   ") == SpannedPart(lit(U"\t  whitespace   ", 0)));
}

TEST_CASE("Alternatives", "[regex]") {
    CHECK(
        parse_regex(U"foo|bar")
        == SpannedPart(
            part::Alternatives(vec<SpannedPart>(lit(U"foo", 0), lit(U"bar", 4))),
            whole(7)));
    CHECK(
        parse_regex(U"foo|x")
        == SpannedPart(
            part::Alternatives(vec<SpannedPart>(lit(U"foo", 0), lit_char(U'x', 4))),
            whole(5)));
    CHECK(
        parse_regex(U"a|b|c|test|d|e")
        == SpannedPart(
            part::Alternatives(vec<SpannedPart>(
                lit_char(U'a', 0),
                lit_char(U'b', 2),
                lit_char(U'c', 4),
                lit(U"test", 6),
                lit_char(U'd', 11),
                lit_char(U'e', 13))),
            whole(14)));
    CHECK(
        parse_regex(U"a||b")
        == SpannedPart(
            part::Alternatives(vec<SpannedPart>(lit_char(U'a', 0), empty(2), lit_char(U'b', 3))),
            whole(4)));
    CHECK(
        parse_regex(U"a|b|")
        == SpannedPart(
            part::Alternatives(vec<SpannedPart>(lit_char(U'a', 0), lit_char(U'b', 2), empty(4))),
            whole(4)));
    CHECK(
        parse_regex(U"|a|b")
        == SpannedPart(
            part::Alternatives(vec<SpannedPart>(empty(0), lit_char(U'a', 1), lit_char(U'b', 3))),
            whole(4)));
    CHECK(
        parse_regex(U"|")
        == SpannedPart(part::Alternatives(vec<SpannedPart>(empty(0), empty(1))), whole(1)));
    CHECK(
        parse_regex(U"|||")
        == SpannedPart(
            part::Alternatives(vec<SpannedPart>(empty(0), empty(1), empty(2), empty(3))),
            whole(3)));
}

TEST_CASE("Groups", "[regex]") {
    CHECK(
        parse_regex(U"(aaa)")
        == SpannedPart(part::Group(capture::Index(), lit(U"aaa", 1)), whole(5)));
    CHECK(
        parse_regex(U"(?:foobar)")
        == SpannedPart(part::Group(capture::None(), lit(U"foobar", 3)), whole(10)));
    CHECK(
        parse_regex(U"(?<x>foobar)")
        == SpannedPart(
            part::Group(capture::Name("x", NamedCaptureFlavor::Angles), lit(U"foobar", 5)),
            whole(12)));
    CHECK(
        parse_regex(U"(?<quux>12345)")
        == SpannedPart(
            part::Group(capture::Name("quux", NamedCaptureFlavor::Angles), lit(U"12345", 8)),
            whole(14)));
    CHECK(
        parse_regex(U"(?'abc123'xyz)")
        == SpannedPart(
            part::Group(capture::Name("abc123", NamedCaptureFlavor::Apostrophes), lit(U"xyz", 10)),
            whole(14)));
    CHECK(
        parse_regex(U"(?P<name>group)")
        == SpannedPart(
            part::Group(capture::Name("name", NamedCaptureFlavor::AnglesWithP), lit(U"group", 9)),
            whole(15)));
    CHECK(
        parse_regex(U"(?P<тест>юникода)")
        == SpannedPart(
            part::Group(capture::Name("тест", NamedCaptureFlavor::AnglesWithP), lit(U"юникода", 9)),
            whole(17)));
    CHECK(parse_regex(U"()") == SpannedPart(part::Group(capture::Index(), empty(1)), whole(2)));
    CHECK(parse_regex(U"(?:)") == SpannedPart(part::Group(capture::None(), empty(3)), whole(4)));
    CHECK(
        parse_regex(U"(?'bb')")
        == SpannedPart(
            part::Group(capture::Name("bb", NamedCaptureFlavor::Apostrophes), empty(6)),
            whole(7)));
    CHECK(
        parse_regex(U"(?P<ccc>)")
        == SpannedPart(
            part::Group(capture::Name("ccc", NamedCaptureFlavor::AnglesWithP), empty(8)),
            whole(9)));

    CHECK_THROWS_MATCHES(
        parse_regex(U"(a"),
        UnexpectedEnd,
        Predicate<UnexpectedEnd>([](const auto& e) { return e.position() == 2; }));
    CHECK_THROWS_MATCHES(
        parse_regex(U"(?a)"),
        UnexpectedChar,
        Predicate<UnexpectedChar>(
            [](const auto& e) { return e.position() == 2 && e.char_got() == U'a'; }));
    CHECK_THROWS_MATCHES(
        parse_regex(U"(?P)"),
        UnexpectedChar,
        Predicate<UnexpectedChar>(
            [](const auto& e) { return e.position() == 3 && e.char_got() == U')'; }));
    CHECK_THROWS_MATCHES(parse_regex(U")"), ExpectedEnd, Predicate<ExpectedEnd>([](const auto& e) {
                             return e.position() == 0 && e.char_got() == U')';
                         }));
    CHECK_THROWS_MATCHES(
        parse_regex(U"(?P'a')"),
        UnexpectedChar,
        Predicate<UnexpectedChar>(
            [](const auto& e) { return e.position() == 3 && e.char_got() == U'\''; }));
    CHECK_THROWS_MATCHES(
        parse_regex(U"(?'a)"),
        UnexpectedChar,
        Predicate<UnexpectedChar>(
            [](const auto& e) { return e.position() == 4 && e.char_got() == U')'; }));
    CHECK_THROWS_MATCHES(
        parse_regex(U"(?<a)"),
        UnexpectedChar,
        Predicate<UnexpectedChar>(
            [](const auto& e) { return e.position() == 4 && e.char_got() == U')'; }));
    CHECK_THROWS_MATCHES(
        parse_regex(U"(?>)"),
        UnexpectedChar,
        Predicate<UnexpectedChar>(
            [](const auto& e) { return e.position() == 2 && e.char_got() == U'>'; }));
}

TEST_CASE("Groups with alternatives", "[regex]") {
    CHECK(
        parse_regex(U"aaa|(bbb|ccc)")
        == SpannedPart(
            part::Alternatives(vec<SpannedPart>(
                lit(U"aaa", 0),
                SpannedPart(
                    part::Group(
                        capture::Index(),
                        SpannedPart(
                            part::Alternatives(vec<SpannedPart>(lit(U"bbb", 5), lit(U"ccc", 9))),
                            Span::make_with_length(5, 7))),
                    Span::make_with_length(4, 9)))),
            whole(13)));

    CHECK(
        parse_regex(U"aaa|(?:(ddd|bbb)zzz|ccc)")
        == SpannedPart(
            part::Alternatives(vec<SpannedPart>(
                lit(U"aaa", 0),
                SpannedPart(
                    part::Group(
                        capture::None(),
                        SpannedPart(
                            part::Alternatives(vec<SpannedPart>(
                                SpannedPart(
                                    part::Sequence(vec<SpannedPart>(
                                        SpannedPart(
                                            part::Group(
                                                capture::Index(),
                                                SpannedPart(
                                                    part::Alternatives(vec<SpannedPart>(
                                                        lit(U"ddd", 8),
                                                        lit(U"bbb", 12))),
                                                    Span::make_with_length(8, 7))),
                                            Span::make_with_length(7, 9)),
                                        lit_char(U'z', 16),
                                        lit_char(U'z', 17),
                                        lit_char(U'z', 18))),
                                    Span::make_with_length(7, 12)),
                                lit(U"ccc", 20))),
                            Span::make_with_length(7, 16))),
                    Span::make_with_length(4, 20)))),
            whole(24)));
}

TEST_CASE("Quantifiers", "[regex]") {
    CHECK(parse_regex(U"z?") == SpannedPart(part::Optional(lit_char(U'z', 0)), whole(2)));
    CHECK(parse_regex(U"z+") == SpannedPart(part::Plus(lit_char(U'z', 0)), whole(2)));
    CHECK(parse_regex(U"z*") == SpannedPart(part::Star(lit_char(U'z', 0)), whole(2)));
    CHECK(parse_regex(U"ц?") == SpannedPart(part::Optional(lit_char(U'ц', 0)), whole(2)));
    CHECK(parse_regex(U"ц+") == SpannedPart(part::Plus(lit_char(U'ц', 0)), whole(2)));
    CHECK(parse_regex(U"ц*") == SpannedPart(part::Star(lit_char(U'ц', 0)), whole(2)));
    CHECK(
        parse_regex(U"()?")
        == SpannedPart(
            part::Optional(
                SpannedPart(part::Group(capture::Index(), empty(1)), Span::make_with_length(0, 2))),
            whole(3)));
    CHECK(
        parse_regex(U"abc?")
        == SpannedPart(
            part::Sequence(vec<SpannedPart>(
                lit_char(U'a', 0),
                lit_char(U'b', 1),
                SpannedPart(part::Optional(lit_char(U'c', 2)), Span::make_with_length(2, 2)))),
            whole(4)));
    CHECK(
        parse_regex(U"a|b?")
        == SpannedPart(
            part::Alternatives(vec<SpannedPart>(
                lit_char(U'a', 0),
                SpannedPart(part::Optional(lit_char(U'b', 2)), Span::make_with_length(2, 2)))),
            whole(4)));
    CHECK(
        parse_regex(U"a?|b?")
        == SpannedPart(
            part::Alternatives(vec<SpannedPart>(
                SpannedPart(part::Optional(lit_char(U'a', 0)), Span::make_with_length(0, 2)),
                SpannedPart(part::Optional(lit_char(U'b', 3)), Span::make_with_length(3, 2)))),
            whole(5)));
    CHECK(
        parse_regex(U"a+|b*")
        == SpannedPart(
            part::Alternatives(vec<SpannedPart>(
                SpannedPart(part::Plus(lit_char(U'a', 0)), Span::make_with_length(0, 2)),
                SpannedPart(part::Star(lit_char(U'b', 3)), Span::make_with_length(3, 2)))),
            whole(5)));
    CHECK(
        parse_regex(U"(a*b+)?|bar*")
        == SpannedPart(
            part::Alternatives(vec<SpannedPart>(
                SpannedPart(
                    part::Optional(SpannedPart(
                        part::Group(
                            capture::Index(),
                            SpannedPart(
                                part::Sequence(vec<SpannedPart>(
                                    SpannedPart(
                                        part::Star(lit_char(U'a', 1)),
                                        Span::make_with_length(1, 2)),
                                    SpannedPart(
                                        part::Plus(lit_char(U'b', 3)),
                                        Span::make_with_length(3, 2)))),
                                Span::make_with_length(1, 4))),
                        Span::make_with_length(0, 6))),
                    Span::make_with_length(0, 7)),
                SpannedPart(
                    part::Sequence(vec<SpannedPart>(
                        lit_char(U'b', 8),
                        lit_char(U'a', 9),
                        SpannedPart(part::Star(lit_char(U'r', 10)), Span::make_with_length(10, 2)))),
                    Span::make_with_length(8, 4)))),
            whole(12)));
    CHECK(
        parse_regex(U"(?:a?)?")
        == SpannedPart(
            part::Optional(SpannedPart(
                part::Group(
                    capture::None(),
                    SpannedPart(part::Optional(lit_char(U'a', 3)), Span::make_with_length(3, 2))),
                Span::make_with_length(0, 6))),
            whole(7)));
    CHECK(
        parse_regex(U"(a?)?")
        == SpannedPart(
            part::Optional(SpannedPart(
                part::Group(
                    capture::Index(),
                    SpannedPart(part::Optional(lit_char(U'a', 1)), Span::make_with_length(1, 2))),
                Span::make_with_length(0, 4))),
            whole(5)));

    CHECK_THROWS_MATCHES(
        parse_regex(U"a???"),
        ExpectedEnd,
        Predicate<ExpectedEnd>([](const auto& e) {
            // TODO: test the correct behavior for lazy quantifiers.
            return e.position() == 2 && e.char_got() == U'?';
        }));
    CHECK_THROWS_MATCHES(
        parse_regex(U"a?++"),
        ExpectedEnd,
        Predicate<ExpectedEnd>([](const auto& e) {
            // TODO: test the correct behavior for possessive quantifiers.
            return e.position() == 2 && e.char_got() == U'+';
        }));
    CHECK_THROWS_MATCHES(
        parse_regex(U"i+++"),
        ExpectedEnd,
        Predicate<ExpectedEnd>([](const auto& e) {
            // TODO: test the correct behavior for possessive quantifiers.
            return e.position() == 2 && e.char_got() == U'+';
        }));
    CHECK_THROWS_MATCHES(parse_regex(U"i+*"), ExpectedEnd, Predicate<ExpectedEnd>([](const auto& e) {
                             return e.position() == 2 && e.char_got() == U'*';
                         }));
    CHECK_THROWS_MATCHES(parse_regex(U"i*?"), ExpectedEnd, Predicate<ExpectedEnd>([](const auto& e) {
                             return e.position() == 2 && e.char_got() == U'?';
                         }));
    CHECK_THROWS_MATCHES(
        parse_regex(U"?"),
        UnexpectedChar,
        Predicate<UnexpectedChar>(
            [](const auto& e) { return e.position() == 0 && e.char_got() == U'?'; }));
    CHECK_THROWS_MATCHES(
        parse_regex(U"*"),
        UnexpectedChar,
        Predicate<UnexpectedChar>(
            [](const auto& e) { return e.position() == 0 && e.char_got() == U'*'; }));
    CHECK_THROWS_MATCHES(
        parse_regex(U"+"),
        UnexpectedChar,
        Predicate<UnexpectedChar>(
            [](const auto& e) { return e.position() == 0 && e.char_got() == U'+'; }));
    CHECK_THROWS_MATCHES(
        parse_regex(U"(?)"),
        UnexpectedChar,
        Predicate<UnexpectedChar>(
            [](const auto& e) { return e.position() == 2 && e.char_got() == U')'; }));
    CHECK_THROWS_MATCHES(
        parse_regex(U"(*)"),
        UnexpectedChar,
        Predicate<UnexpectedChar>(
            [](const auto& e) { return e.position() == 1 && e.char_got() == U'*'; }));
    CHECK_THROWS_MATCHES(
        parse_regex(U"(+)"),
        UnexpectedChar,
        Predicate<UnexpectedChar>(
            [](const auto& e) { return e.position() == 1 && e.char_got() == U'+'; }));
    CHECK_THROWS_MATCHES(
        parse_regex(U"(++)"),
        UnexpectedChar,
        Predicate<UnexpectedChar>(
            [](const auto& e) { return e.position() == 1 && e.char_got() == U'+'; }));
    CHECK_THROWS_MATCHES(
        parse_regex(U"??"),
        UnexpectedChar,
        Predicate<UnexpectedChar>(
            [](const auto& e) { return e.position() == 0 && e.char_got() == U'?'; }));
    CHECK_THROWS_MATCHES(
        parse_regex(U"*+"),
        UnexpectedChar,
        Predicate<UnexpectedChar>(
            [](const auto& e) { return e.position() == 0 && e.char_got() == U'*'; }));
}

TEST_CASE("Wildcard", "[regex]") {
    CHECK(parse_regex(U".") == SpannedPart(part::Wildcard(), whole(1)));
    CHECK(
        parse_regex(U"a.b")
        == SpannedPart(
            part::Sequence(vec<SpannedPart>(
                lit_char(U'a', 0),
                SpannedPart(part::Wildcard(), Span::make_with_length(1, 1)),
                lit_char(U'b', 2))),
            whole(3)));
    CHECK(
        parse_regex(U".?")
        == SpannedPart(
            part::Optional(SpannedPart(part::Wildcard(), Span::make_with_length(0, 1))),
            whole(2)));
}

TEST_CASE("Sequences with groups", "[regex]") {
    CHECK(
        parse_regex(U"a(b)")
        == SpannedPart(
            part::Sequence(vec<SpannedPart>(
                lit_char(U'a', 0),
                SpannedPart(
                    part::Group(capture::Index(), lit_char(U'b', 2)),
                    Span::make_with_length(1, 3)))),
            whole(4)));
}
