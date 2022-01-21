#pragma once

// STL
#include <exception>
#include <stdexcept>

namespace wr22::regex_parser::parser::errors {

struct ParseError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class UnexpectedEnd : public ParseError {
public:
    UnexpectedEnd(size_t position, std::string expected);

    size_t position() const;
    const std::string& expected() const;

private:
    size_t m_position;
    std::string m_expected;
};

class ExpectedEnd : public ParseError {
public:
    ExpectedEnd(size_t position, char32_t char_got);

    size_t position() const;
    char32_t char_got() const;

private:
    size_t m_position;
    char32_t m_char_got;
};

}
