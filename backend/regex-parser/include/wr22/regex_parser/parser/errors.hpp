#pragma once

// STL
#include <exception>
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
    UnexpectedEnd(size_t position, std::string expected);

    /// Get the input position. See the constructor docs for a more detailed description.
    size_t position() const;
    /// Get the description of expected characters. See the constructor docs for a more detailed
    /// description.
    const std::string& expected() const;

private:
    size_t m_position;
    std::string m_expected;
};

/// The error when the parser expected the input to end, but it did not.
class ExpectedEnd : public ParseError {
public:
    /// Constructor.
    ///
    /// @param position the 0-based position in the input when the parser has encountered the end of
    /// input.
    /// @char_got the character that the parser has received instead of the end of input.
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

}  // namespace wr22::regex_parser::parser::errors
