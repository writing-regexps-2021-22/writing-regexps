// wr22
#include <wr22/regex_parser/parser/errors.hpp>
#include <wr22/regex_parser/parser/regex.hpp>
#include <wr22/regex_parser/regex/part.hpp>

// stl
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace wr22::regex_parser::parser {

template <typename Iter, typename Sentinel>
requires requires(Iter iter, Sentinel end) {
    ++iter;
    { *iter } -> std::convertible_to<char32_t>;
    { iter == end } -> std::convertible_to<bool>;
    { iter != end } -> std::convertible_to<bool>;
}
class Parser {
public:
    Parser(Iter begin, Sentinel end) : m_iter(begin), m_end(end) {}

    void expect_end() {
        if (m_iter != m_end) {
            throw errors::ExpectedEnd(m_pos, *m_iter);
        }
    }

    regex::Part parse_regex() {
        return parse_alternatives();
    }

    regex::Part parse_alternatives() {
        auto part = parse_sequence();
        // Either this is the only alternative (in which case we flatten the AST and return the only
        // alternative itself)...
        if (lookahead() != U'|') {
            return part;
        }

        // ... or we will be constructing a vector of alternatives.
        // Note: we cannot use an initializer list, because this would
        // entail copying `part`, whose copy constructor is deleted.
        std::vector<regex::Part> alternatives;
        alternatives.push_back(std::move(part));
        while (lookahead() == U'|') {
            advance(1, "`|`");
            alternatives.push_back(parse_sequence());
        }
        return regex::part::Alternatives{std::move(alternatives)};
    }

    regex::Part parse_sequence() {
        auto part = parse_atom();
        if (!can_start_atom(lookahead())) {
            return part;
        }

        std::vector<regex::Part> items;
        items.push_back(std::move(part));
        while (can_start_atom(lookahead())) {
            items.push_back(parse_atom());
        }
        return regex::part::Sequence{std::move(items)};
    }

    regex::Part parse_atom() {
        auto la = lookahead();
        if (la == U'(') {
            return parse_group();
        } else {
            return parse_char_literal();
        }
    }

    regex::Part parse_char_literal() {
        auto c = next_char();
        if (!c.has_value()) {
            throw errors::UnexpectedEnd(m_pos, "a character");
        }
        return regex::part::Literal{c.value()};
    }

    regex::Part parse_group() {
        throw std::runtime_error("Groups are not implemented");
    }

private:
    std::optional<char32_t> lookahead() {
        if (m_iter == m_end) {
            return std::nullopt;
        }
        return *m_iter;
    }

    std::optional<char32_t> next_char() {
        if (m_iter == m_end) {
            return std::nullopt;
        }
        auto c = *m_iter;
        ++m_iter;
        ++m_pos;
        return c;
    }

    void advance(size_t num_skipped_chars, std::string expected) {
        for (size_t i = 0; i < num_skipped_chars; ++i) {
            if (m_iter == m_end) {
                throw errors::UnexpectedEnd(m_pos, std::move(expected));
            }
            ++m_iter;
            ++m_pos;
        }
    }

    static bool can_start_atom(char32_t c) {
        for (auto forbidden_char : std::basic_string_view(U"[](){}|?+*")) {
            if (c == forbidden_char) {
                return false;
            }
        }
        return true;
    }

    static bool can_start_atom(std::optional<char32_t> c) {
        return c.has_value() && can_start_atom(c.value());
    }

    Iter m_iter;
    Sentinel m_end;
    size_t m_pos = 0;
};

template <typename Iter, typename Sentinel>
Parser(Iter begin, Sentinel end) -> Parser<Iter, Sentinel>;

regex::Part parse_regex(const utils::UnicodeStringView& regex) {
    auto parser = Parser(regex.begin(), regex.end());
    auto result = parser.parse_regex();
    parser.expect_end();
    return result;
}

}  // namespace wr22::regex_parser::parser
