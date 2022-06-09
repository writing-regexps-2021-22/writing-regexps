#pragma once

// wr22
#include <wr22/regex_parser/span/span.hpp>

// STL
#include <exception>
#include <optional>
#include <stdexcept>
#include <string>

namespace wr22::regex_parser::parser::errors {

/// The base class for parse errors.
///
/// This exception type should be caught if it is desired to catch all parse errors. However, there
/// are more specific exceptions deriving from this one that can be handled separately for greater
/// flexibility.
struct ParseError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

/// The error when the parser hit the end of the input earlier than it expected.
class UnexpectedEnd : public ParseError {
public:
    /// Constructor.
    ///
    /// @param position the 0-based position in the input when the parser has encountered the end of
    /// input.
    /// @param expected a textual description of a class of characters expected instead.
    /// @param needs_closing a character describing the type of bracket that needs to be closed, if
    /// any.
    UnexpectedEnd(size_t position, std::string expected, std::optional<char32_t> needs_closing);

    /// Get the input position. See the constructor docs for a more detailed description.
    size_t position() const;
    /// Get the description of expected characters. See the constructor docs for a more detailed
    /// description.
    const std::string& expected() const;
    /// Get the bracket that needs to be closed.
    std::optional<char32_t> needs_closing() const;

private:
    size_t m_position;
    std::string m_expected;
    std::optional<char32_t> m_needs_closing;
};

/// The error when the parser expected the input to end, but it did not.
class ExpectedEnd : public ParseError {
public:
    /// Constructor.
    ///
    /// @param position the 0-based position in the input when the parser has encountered the end of
    /// input.
    /// @param char_got the character that the parser has received instead of the end of input.
    ExpectedEnd(size_t position, char32_t char_got);

    /// Get the input position. See the constructor docs for a more detailed description.
    size_t position() const;
    /// Get the character the parser has received. See the constructor docs for a more detailed
    /// description.
    char32_t char_got() const;

private:
    size_t m_position;
    char32_t m_char_got;
};

/// The error when the parser got a character it didn't expect at the current position.
class UnexpectedChar : public ParseError {
public:
    /// Constructor.
    ///
    /// @param position the 0-based position in the input when the parser has encountered the
    /// unexpected character.
    /// @param char_got the character that the parser has received.
    /// @param expected a textual description of a class of characters expected instead.
    /// @param needs_closing a character describing the type of bracket that needs to be closed, if
    /// any.
    UnexpectedChar(
        size_t position,
        char32_t char_got,
        std::string expected,
        std::optional<char32_t> needs_closing);

    /// Get the input position. See the constructor docs for a more detailed description.
    size_t position() const;
    /// Get the character the parser has received. See the constructor docs for a more detailed
    /// description.
    char32_t char_got() const;
    /// Get the description of expected characters. See the constructor docs for a more detailed
    /// description.
    const std::string& expected() const;
    /// Get the bracket that needs to be closed.
    std::optional<char32_t> needs_closing() const;

private:
    size_t m_position;
    char32_t m_char_got;
    std::string m_expected;
    std::optional<char32_t> m_needs_closing;
};

/// The error indicating that a character range in a character class is invalid.
class InvalidRange : public ParseError {
public:
    /// Constructor.
    ///
    /// @param span the span of the character range considered.
    /// @param first the first character in (left bound of) the range, as in the regex.
    /// @param last the last character in (right bound of) the range, as in the regex.
    InvalidRange(span::Span span, char32_t first, char32_t last);

    /// Get the span of the character range. See the constructor docs for a more detailed explanation.
    span::Span span() const;
    /// Get the first character in the character range. See the constructor docs for a more detailed
    /// explanation.
    char32_t first() const;
    /// Get the last character in the character range. See the constructor docs for a more detailed
    /// explanation.
    char32_t last() const;

private:
    span::Span m_span;
    char32_t m_first;
    char32_t m_last;
};

/// The error signalling that the regular expression has too many levels of nesting to be parsed.
class TooStronglyNested : public ParseError {
public:
    /// Constructor.
    TooStronglyNested();
};

}  // namespace wr22::regex_parser::parser::errors
