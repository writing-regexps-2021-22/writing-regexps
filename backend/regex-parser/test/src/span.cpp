// catch2
#include <catch2/catch.hpp>

// wr22
#include <limits>
#include <wr22/regex_parser/span/span.hpp>

// stl
#include <type_traits>

using wr22::regex_parser::span::InvalidSpan;
using wr22::regex_parser::span::Span;

TEST_CASE("Span::make_empty works", "[span]") {
    {
        auto span = Span::make_empty(0);
        CHECK(span.begin() == 0);
        CHECK(span.end() == 0);
    }
    {
        auto span = Span::make_empty(100);
        CHECK(span.begin() == 100);
        CHECK(span.end() == 100);
    }
    auto max = std::numeric_limits<size_t>::max();
    {
        auto span = Span::make_empty(max);
        CHECK(span.begin() == max);
        CHECK(span.end() == max);
    }
}

TEST_CASE("Span::make_single_position works", "[span]") {
    {
        auto span = Span::make_single_position(3);
        CHECK(span.begin() == 3);
        CHECK(span.end() == 4);
    }
    {
        auto span = Span::make_single_position(0);
        CHECK(span.begin() == 0);
        CHECK(span.end() == 1);
    }

    auto max = std::numeric_limits<size_t>::max();
    {
        auto span = Span::make_single_position(max - 1);
        CHECK(span.begin() == max - 1);
        CHECK(span.end() == max);
    }
    CHECK_THROWS_AS(Span::make_single_position(max), InvalidSpan);
}

TEST_CASE("Span::make_from_positions works", "[span]") {
    {
        auto span = Span::make_from_positions(0, 1);
        CHECK(span.begin() == 0);
        CHECK(span.end() == 1);
    }
    {
        auto span = Span::make_from_positions(2, 5);
        CHECK(span.begin() == 2);
        CHECK(span.end() == 5);
    }
    {
        auto span = Span::make_from_positions(1337, 98765);
        CHECK(span.begin() == 1337);
        CHECK(span.end() == 98765);
    }
    {
        auto span = Span::make_from_positions(98764, 98765);
        CHECK(span.begin() == 98764);
        CHECK(span.end() == 98765);
    }
    {
        auto span = Span::make_from_positions(0, 0);
        CHECK(span.begin() == 0);
        CHECK(span.end() == 0);
    }
    {
        auto span = Span::make_from_positions(12345, 12345);
        CHECK(span.begin() == 12345);
        CHECK(span.end() == 12345);
    }
    {
        auto span = Span::make_from_positions(98765, 98765);
        CHECK(span.begin() == 98765);
        CHECK(span.end() == 98765);
    }
    CHECK_THROWS_AS(Span::make_from_positions(98765, 98065), InvalidSpan);
    CHECK_THROWS_AS(Span::make_from_positions(98765, 0), InvalidSpan);
}

TEST_CASE("Span::make_with_length works", "[span]") {
    {
        auto span = Span::make_with_length(0, 1);
        CHECK(span.begin() == 0);
        CHECK(span.end() == 1);
    }
    {
        auto span = Span::make_with_length(0, 5);
        CHECK(span.begin() == 0);
        CHECK(span.end() == 5);
    }
    {
        auto span = Span::make_with_length(17, 5);
        CHECK(span.begin() == 17);
        CHECK(span.end() == 22);
    }
    {
        auto span = Span::make_with_length(0, 0);
        CHECK(span.begin() == 0);
        CHECK(span.end() == 0);
    }
    {
        auto span = Span::make_with_length(17, 0);
        CHECK(span.begin() == 17);
        CHECK(span.end() == 17);
    }

    auto max = std::numeric_limits<size_t>::max();
    {
        auto span = Span::make_with_length(0, max);
        CHECK(span.begin() == 0);
        CHECK(span.end() == max);
    }
    {
        auto span = Span::make_with_length(1000, max - 1000);
        CHECK(span.begin() == 1000);
        CHECK(span.end() == max);
    }
    {
        auto span = Span::make_with_length(888, 0);
        CHECK(span.begin() == 888);
        CHECK(span.end() == 888);
    }
    {
        auto span = Span::make_with_length(max, 0);
        CHECK(span.begin() == max);
        CHECK(span.end() == max);
    }
    CHECK_THROWS_AS(Span::make_with_length(1, max), InvalidSpan);
    CHECK_THROWS_AS(Span::make_with_length(max, 1), InvalidSpan);
    CHECK_THROWS_AS(Span::make_with_length(max - 1, 2), InvalidSpan);
    CHECK_THROWS_AS(Span::make_with_length(max - 1, 100), InvalidSpan);
    CHECK_THROWS_AS(Span::make_with_length(max, max), InvalidSpan);
}

TEST_CASE("Span::length works", "[span]") {
    CHECK(Span::make_empty(0).length() == 0);
    CHECK(Span::make_empty(5).length() == 0);
    CHECK(Span::make_empty(1000).length() == 0);
    CHECK(Span::make_single_position(3).length() == 1);
    CHECK(Span::make_single_position(100).length() == 1);
    CHECK(Span::make_from_positions(3, 8).length() == 5);
    CHECK(Span::make_from_positions(111, 112).length() == 1);
    CHECK(Span::make_from_positions(111, 115).length() == 4);
    CHECK(Span::make_from_positions(115, 115).length() == 0);
    CHECK(Span::make_with_length(0, 115).length() == 115);
    CHECK(Span::make_with_length(115, 0).length() == 0);
    CHECK(Span::make_with_length(120, 115).length() == 115);
    CHECK(Span::make_with_length(120, 1).length() == 1);

    auto max = std::numeric_limits<size_t>::max();
    CHECK(Span::make_empty(max).length() == 0);
    CHECK(Span::make_from_positions(111, max).length() == max - 111);
    CHECK(Span::make_from_positions(0, max).length() == max);
    CHECK(Span::make_with_length(max - 5, 2).length() == 2);
    CHECK(Span::make_with_length(max - 5, 5).length() == 5);
    CHECK(Span::make_with_length(0, max).length() == max);
    CHECK(Span::make_with_length(100, max - 100).length() == max - 100);
    CHECK(Span::make_with_length(100, max - 101).length() == max - 101);
}
