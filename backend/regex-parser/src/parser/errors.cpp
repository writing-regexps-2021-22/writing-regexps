// wr22
#include <wr22/regex_parser/parser/errors.hpp>

// STL
#include <fmt/core.h>

// boost
#include <boost/locale/encoding_utf.hpp>

namespace wr22::regex_parser::parser::errors {

namespace {
    std::string char_to_utf8(char32_t c) {
        return boost::locale::conv::utf_to_utf<char>(&c, &c + 1);
    }
}  // namespace

UnexpectedEnd::UnexpectedEnd(size_t position, std::string expected)
    : ParseError(
        fmt::format("Unexpected end of input at position {}: expected {}", position, expected)),
      m_position(position), m_expected(std::move(expected)) {}

size_t UnexpectedEnd::position() const {
    return m_position;
}

const std::string& UnexpectedEnd::expected() const {
    return m_expected;
}

ExpectedEnd::ExpectedEnd(size_t position, char32_t char_got)
    : ParseError(fmt::format(
        "Expected the input to end at position {}, but got the character `{}` (U+{:x})",
        position,
        char_to_utf8(char_got),
        static_cast<uint32_t>(char_got))),
      m_position(position), m_char_got(char_got) {}

size_t ExpectedEnd::position() const {
    return m_position;
}

char32_t ExpectedEnd::char_got() const {
    return m_char_got;
}

UnexpectedChar::UnexpectedChar(size_t position, char32_t char_got, std::string expected)
    : ParseError(fmt::format(
        "Expected {}, but got the character `{}` (U+{:x}) at position {}",
        expected,
        char_to_utf8(char_got),
        static_cast<uint32_t>(char_got),
        position)),
      m_position(position), m_char_got(char_got), m_expected(std::move(expected)) {}

size_t UnexpectedChar::position() const {
    return m_position;
}

char32_t UnexpectedChar::char_got() const {
    return m_char_got;
}

const std::string& UnexpectedChar::expected() const {
    return m_expected;
}

}  // namespace wr22::regex_parser::parser::errors
