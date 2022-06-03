// catch
#include <catch2/catch.hpp>

// wr22
#include <wr22/regex_executor/executor.hpp>
#include <wr22/regex_executor/regex.hpp>
#include <wr22/regex_parser/parser/regex.hpp>

// stl
#include <iostream>

using wr22::regex_executor::Executor;
using wr22::regex_executor::Regex;
using wr22::regex_parser::parser::parse_regex;

TEST_CASE("Basic star quantifier works") {
    auto regex = Regex(parse_regex(U"(.*)ll"));
    auto ex = Executor(regex);
    CHECK(ex.execute(U"ball").matched);
    CHECK(ex.execute(U"ll").matched);
    CHECK(ex.execute(U"lllll").matched);
    CHECK(!ex.execute(U"llu").matched);
    CHECK(!ex.execute(U"ballpark").matched);
    CHECK(!ex.execute(U"sol").matched);
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
    CHECK(!ex.execute(U"ac").matched);
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
    CHECK(!ex.execute(U"abbc").matched);
    CHECK(!ex.execute(U"abbbc").matched);
    CHECK(!ex.execute(U"abbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbc").matched);
}

TEST_CASE("Nested quantifiers work") {
    auto regex = Regex(parse_regex(U"a(?:b+c)+d"));
    auto ex = Executor(regex);
    CHECK(!ex.execute(U"ad").matched);
    CHECK(ex.execute(U"abcd").matched);
    CHECK(ex.execute(U"abbcd").matched);
    CHECK(ex.execute(U"abbcbcd").matched);
    CHECK(ex.execute(U"abcbcd").matched);
    CHECK(ex.execute(U"abcbcbcbbcd").matched);
    CHECK(!ex.execute(U"abbcbce").matched);
}

TEST_CASE("Alternatives work") {
    auto regex = Regex(parse_regex(U"a(?:b|c)d"));
    auto ex = Executor(regex);
    CHECK(!ex.execute(U"ad").matched);
    CHECK(!ex.execute(U"ab").matched);
    CHECK(!ex.execute(U"ac").matched);
    CHECK(!ex.execute(U"abc").matched);
    CHECK(!ex.execute(U"abcd").matched);
    CHECK(!ex.execute(U"abbd").matched);
    CHECK(ex.execute(U"abd").matched);
    CHECK(ex.execute(U"acd").matched);
}

TEST_CASE("Alternatives nested into quantifiers work") {
    auto regex = Regex(parse_regex(U"a(?:b|bbc)*d"));
    auto ex = Executor(regex);
    CHECK(ex.execute(U"ad").matched);
    CHECK(!ex.execute(U"ab").matched);
    CHECK(!ex.execute(U"abbc").matched);
    CHECK(ex.execute(U"abbcd").matched);
    CHECK(ex.execute(U"abbbcd").matched);
    CHECK(ex.execute(U"abbbbcd").matched);
    CHECK(ex.execute(U"abd").matched);
    CHECK(ex.execute(U"abbd").matched);
    CHECK(ex.execute(U"abbbbd").matched);
    CHECK(!ex.execute(U"abcd").matched);
}

TEST_CASE("Quantifiers nested into alternatives work") {
    auto regex = Regex(parse_regex(U"a(?:b+c|cc)*d"));
    auto ex = Executor(regex);
    CHECK(ex.execute(U"ad").matched);
    CHECK(ex.execute(U"abbbcd").matched);
    CHECK(ex.execute(U"abbbcccd").matched);
    CHECK(!ex.execute(U"abbbccd").matched);
}
