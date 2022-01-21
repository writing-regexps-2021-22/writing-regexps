// wr22
#include <wr22/regex_parser/parser/errors.hpp>

// STL
#include <fmt/core.h>

// boost
#include <boost/locale/encoding_utf.hpp>

namespace wr22::regex_parser::parser::errors {

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
        "Expected the input to end at position {}, but got a character `{}` (U+{})",
        position,
        boost::locale::conv::utf_to_utf<char>(&char_got, &char_got + 1),
        static_cast<uint32_t>(char_got))),
      m_position(position), m_char_got(char_got) {}

size_t ExpectedEnd::position() const {
    return m_position;
}

char32_t ExpectedEnd::char_got() const {
    return m_char_got;
}

}  // namespace wr22::regex_parser::parser::errors
