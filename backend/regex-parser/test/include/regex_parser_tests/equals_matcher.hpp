#pragma once

// catch2
#include <catch2/catch.hpp>

// stl
#include <concepts>

// fmt
#include <fmt/core.h>

namespace regex_parser_tests {

/// A Catch2 matcher which checks that a value compares equal to a reference value.
template <typename T>
requires std::equality_comparable<T>
class Equals : public Catch::MatcherBase<T> {
public:
    explicit Equals(T reference_value);

    bool match(const T& value) const override {
        return value == m_reference_value;
    }

    std::string describe() const override {
        return fmt::format("compares equal to {}", m_reference_value);
    }

private:
    T m_reference_value;
};

/// Type deduction guideline for Equals.
template <typename T>
Equals(T reference_value) -> Equals<T>;

}  // namespace regex_parser_tests
