// catch
#include <catch2/catch.hpp>

// wr22
#include <wr22/regex_executor/executor.hpp>
#include <wr22/regex_executor/regex.hpp>
#include <wr22/regex_parser/parser/regex.hpp>

using wr22::regex_executor::Capture;
using wr22::regex_executor::Captures;
using wr22::regex_executor::Executor;
using wr22::regex_executor::Regex;
using wr22::regex_parser::parser::parse_regex;
using wr22::regex_parser::span::Span;

TEST_CASE("Basic star quantifier works") {
    auto regex = Regex(parse_regex(U"(.*)ll"));
    auto ex = Executor(regex);
    CHECK(ex.execute(U"ball").matched);
    CHECK(ex.execute(U"ll").matched);
    CHECK(ex.execute(U"lllll").matched);
    CHECK_FALSE(ex.execute(U"llu").matched);
    CHECK_FALSE(ex.execute(U"ballpark").matched);
    CHECK_FALSE(ex.execute(U"sol").matched);
}

TEST_CASE("Star matches a correct number of items") {
    auto regex = Regex(parse_regex(U"ab*c"));
    auto ex = Executor(regex);
    CHECK(ex.execute(U"ac").matched);
    CHECK(ex.execute(U"abc").matched);
    CHECK(ex.execute(U"abbc").matched);
    CHECK(ex.execute(U"abbbc").matched);
    CHECK(ex.execute(U"abbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbc").matched);
}

TEST_CASE("Plus matches a correct number of items") {
    auto regex = Regex(parse_regex(U"ab+c"));
    auto ex = Executor(regex);
    CHECK_FALSE(ex.execute(U"ac").matched);
    CHECK(ex.execute(U"abc").matched);
    CHECK(ex.execute(U"abbc").matched);
    CHECK(ex.execute(U"abbbc").matched);
    CHECK(ex.execute(U"abbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbc").matched);
}

TEST_CASE("Optional matches a correct number of items") {
    auto regex = Regex(parse_regex(U"ab?c"));
    auto ex = Executor(regex);
    CHECK(ex.execute(U"ac").matched);
    CHECK(ex.execute(U"abc").matched);
    CHECK_FALSE(ex.execute(U"abbc").matched);
    CHECK_FALSE(ex.execute(U"abbbc").matched);
    CHECK_FALSE(ex.execute(U"abbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbc").matched);
}

TEST_CASE("Nested quantifiers work") {
    auto regex = Regex(parse_regex(U"a(?:b+c)+d"));
    auto ex = Executor(regex);
    CHECK_FALSE(ex.execute(U"ad").matched);
    CHECK(ex.execute(U"abcd").matched);
    CHECK(ex.execute(U"abbcd").matched);
    CHECK(ex.execute(U"abbcbcd").matched);
    CHECK(ex.execute(U"abcbcd").matched);
    CHECK(ex.execute(U"abcbcbcbbcd").matched);
    CHECK_FALSE(ex.execute(U"abbcbce").matched);
}

TEST_CASE("Alternatives work") {
    auto regex = Regex(parse_regex(U"a(?:b|c)d"));
    auto ex = Executor(regex);
    CHECK_FALSE(ex.execute(U"ad").matched);
    CHECK_FALSE(ex.execute(U"ab").matched);
    CHECK_FALSE(ex.execute(U"ac").matched);
    CHECK_FALSE(ex.execute(U"abc").matched);
    CHECK_FALSE(ex.execute(U"abcd").matched);
    CHECK_FALSE(ex.execute(U"abbd").matched);
    CHECK(ex.execute(U"abd").matched);
    CHECK(ex.execute(U"acd").matched);
}

TEST_CASE("Alternatives nested into quantifiers work") {
    auto regex = Regex(parse_regex(U"a(?:b|bbc)*d"));
    auto ex = Executor(regex);
    CHECK(ex.execute(U"ad").matched);
    CHECK_FALSE(ex.execute(U"ab").matched);
    CHECK_FALSE(ex.execute(U"abbc").matched);
    CHECK(ex.execute(U"abbcd").matched);
    CHECK(ex.execute(U"abbbcd").matched);
    CHECK(ex.execute(U"abbbbcd").matched);
    CHECK(ex.execute(U"abd").matched);
    CHECK(ex.execute(U"abbd").matched);
    CHECK(ex.execute(U"abbbbd").matched);
    CHECK_FALSE(ex.execute(U"abcd").matched);
}

TEST_CASE("Quantifiers nested into alternatives work") {
    auto regex = Regex(parse_regex(U"a(?:b+c|cc)*d"));
    auto ex = Executor(regex);
    CHECK(ex.execute(U"ad").matched);
    CHECK(ex.execute(U"abbbcd").matched);
    CHECK(ex.execute(U"abbbcccd").matched);
    CHECK_FALSE(ex.execute(U"abbbccd").matched);
}

TEST_CASE("Groups capturing by index work") {
    auto regex = Regex(parse_regex(U"(a)(b*)(c)?"));
    auto ex = Executor(regex);
    CHECK(
        ex.execute(U"abbc").captures.value()
        == Captures{
            .whole = Capture{.string_span = Span::make_with_length(0, 4)},
            .indexed =
                {
                    {1, Capture{.string_span = Span::make_with_length(0, 1)}},
                    {2, Capture{.string_span = Span::make_with_length(1, 2)}},
                    {3, Capture{.string_span = Span::make_with_length(3, 1)}},
                },
            .named = {},
        });
    CHECK(
        ex.execute(U"a").captures.value()
        == Captures{
            .whole = Capture{.string_span = Span::make_with_length(0, 1)},
            .indexed =
                {
                    {1, Capture{.string_span = Span::make_with_length(0, 1)}},
                    {2, Capture{.string_span = Span::make_empty(1)}},
                },
            .named = {},
        });
}

TEST_CASE("Non-capturing groups do not capture") {
    auto regex = Regex(parse_regex(U"(?:a)(?:b*)(?:c)?"));
    auto ex = Executor(regex);
    CHECK(
        ex.execute(U"abbc").captures.value()
        == Captures{
            .whole = Capture{.string_span = Span::make_with_length(0, 4)},
            .indexed = {},
            .named = {},
        });
    CHECK(
        ex.execute(U"a").captures.value()
        == Captures{
            .whole = Capture{.string_span = Span::make_with_length(0, 1)},
            .indexed = {},
            .named = {},
        });
}

TEST_CASE("Groups capturing by name work") {
    auto regex = Regex(parse_regex(U"(?P<A>a)(?P<B>b*)(?P<C>c)?"));
    auto ex = Executor(regex);
    CHECK(
        ex.execute(U"abbc").captures.value()
        == Captures{
            .whole = Capture{.string_span = Span::make_with_length(0, 4)},
            .indexed = {},
            .named =
                {
                    {"A", Capture{.string_span = Span::make_with_length(0, 1)}},
                    {"B", Capture{.string_span = Span::make_with_length(1, 2)}},
                    {"C", Capture{.string_span = Span::make_with_length(3, 1)}},
                },
        });
    CHECK(
        ex.execute(U"a").captures.value()
        == Captures{
            .whole = Capture{.string_span = Span::make_with_length(0, 1)},
            .indexed = {},
            .named =
                {
                    {"A", Capture{.string_span = Span::make_with_length(0, 1)}},
                    {"B", Capture{.string_span = Span::make_empty(1)}},
                },
        });
}

TEST_CASE("Repeated group captures by index work") {
    auto regex = Regex(parse_regex(U"(.)*"));
    auto ex = Executor(regex);
    CHECK(
        ex.execute(U"abc").captures.value()
        == Captures{
            .whole = Capture{.string_span = Span::make_with_length(0, 3)},
            .indexed =
                {
                    {1, Capture{.string_span = Span::make_with_length(2, 1)}},
                },
            .named = {},
        });
}

TEST_CASE("Repeated group captures by name work") {
    auto regex = Regex(parse_regex(U"(?P<foo>.)*"));
    auto ex = Executor(regex);
    CHECK(
        ex.execute(U"abc").captures.value()
        == Captures{
            .whole = Capture{.string_span = Span::make_with_length(0, 3)},
            .indexed = {},
            .named =
                {
                    {"foo", Capture{.string_span = Span::make_with_length(2, 1)}},
                },
        });
}

TEST_CASE("Empty group under an unbounded quantifier works") {
    auto regex = Regex(parse_regex(U"(?:)*"));
    auto ex = Executor(regex);
    CHECK(ex.execute(U"").matched);
    CHECK_FALSE(ex.execute(U"a").matched);
}

TEST_CASE("Empty group under a bounded quantifier works") {
    auto regex = Regex(parse_regex(U"(?:)?"));
    auto ex = Executor(regex);
    CHECK(ex.execute(U"").matched);
    CHECK_FALSE(ex.execute(U"a").matched);
}

TEST_CASE("Empty optional under an unbounded quantifier works") {
    auto regex = Regex(parse_regex(U"(?:a?)*b"));
    auto ex = Executor(regex);
    CHECK(ex.execute(U"ab").matched);
    CHECK(ex.execute(U"b").matched);
    CHECK(ex.execute(U"aaaaaab").matched);
    CHECK_FALSE(ex.execute(U"aaa").matched);
    CHECK_FALSE(ex.execute(U"").matched);
}
