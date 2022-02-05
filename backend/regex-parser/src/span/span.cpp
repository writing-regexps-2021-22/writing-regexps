// wr22
#include <stdexcept>
#include <wr22/regex_parser/span/span.hpp>

// fmt
#include <fmt/core.h>
#include <fmt/ostream.h>

namespace wr22::regex_parser::span {

InvalidSpan::InvalidSpan(size_t begin, size_t end)
    : std::runtime_error(fmt::format(
        "The span with begin = {} and end = {} is invalid because end < begin",
        begin,
        end)),
      begin(begin), end(end) {}

Span Span::make_empty(size_t position) {
    return Span::make_from_positions(position, position);
}

Span Span::make_single_position(size_t position) {
    return Span::make_from_positions(position, position + 1);
}

Span Span::make_from_positions(size_t begin, size_t end) {
    return Span(begin, end);
}

Span Span::make_with_length(size_t begin, size_t length) {
    return Span::make_from_positions(begin, begin + length);
}

size_t Span::length() const {
    return m_end - m_begin;
}

size_t Span::begin() const {
    return m_begin;
}

size_t Span::end() const {
    return m_end;
}

Span::Span(size_t begin, size_t end) : m_begin(begin), m_end(end) {
    if (end < begin) {
        throw InvalidSpan(begin, end);
    }
}

std::ostream& operator<<(std::ostream& out, Span span) {
    fmt::print(out, "{}..{}", span.begin(), span.end());
    return out;
}

}  // namespace wr22::regex_parser::span
