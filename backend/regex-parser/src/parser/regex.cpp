// wr22
#include <boost/locale/utf.hpp>
#include <wr22/regex_parser/parser/errors.hpp>
#include <wr22/regex_parser/parser/regex.hpp>
#include <wr22/regex_parser/regex/part.hpp>

// stl
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

// boost
#include <boost/locale/encoding_utf.hpp>

namespace wr22::regex_parser::parser {

namespace {
    void push_utf8(std::string& buf, char32_t unicode_char) {
        using Traits = boost::locale::utf::utf_traits<char>;
        Traits::encode(
            static_cast<boost::locale::utf::code_point>(unicode_char),
            std::back_inserter(buf));
    }
}  // namespace

/// A regex parser.
///
/// For additional information see the methods' docs, particularly the constructor and the
/// `parse_regex` method.
template <typename Iter, typename Sentinel>
requires requires(Iter iter, Sentinel end) {
    ++iter;
    { *iter } -> std::convertible_to<char32_t>;
    { iter == end } -> std::convertible_to<bool>;
    { iter != end } -> std::convertible_to<bool>;
}
class Parser {
public:
    /// Constructor.
    ///
    /// This constructor stores a pair of forward iterators that should generate a sequence of
    /// Unicode code points (`char32_t`). The `begin` iterator and the `end` sentinel may have
    /// different types provided that the iterator can is equality comparable with the sentinel.
    ///
    /// SAFETY:
    /// The iterators must not be invalidated as long as this `Parser` object is still alive.
    Parser(Iter begin, Sentinel end) : m_iter(begin), m_end(end) {}

    /// Ensure that the parser has consumed all of the input.
    ///
    /// Does nothing if all input has been consumed.
    /// @throws errors::ExpectedEnd if this is not the case.
    void expect_end() {
        if (m_iter != m_end) {
            throw errors::ExpectedEnd(m_pos, *m_iter);
        }
    }

    /// Parse a regex consuming part of the remaining input.
    ///
    /// This is **the** method that should be called to parse a regular expression because
    /// it represents the root rule of the regex grammar. Please note that this method may
    /// not consume all of the parser's input. Hence, if a whole regex is to be parsed,
    /// the `expect_end` method should be called afterwards.
    ///
    /// @returns the parsed regex AST (some variant of `regex::Part` depending on the input).
    ///
    /// @throws errors::ParseError if the input cannot be parsed.
    regex::Part parse_regex() {
        // Empty regexes are handled as empty sequences.
        return parse_alternatives();
    }

    /// Intermediate rule: parse a pipe-separated list of alternatives (e.g. `a|bb|ccc`).
    ///
    /// @returns the list of parsed alternatives packed into `regex::part::Alternatives` or, if and
    /// only if the list of alternatives contains exactly 1 element, the only alternative unchanged.
    ///
    /// @throws errors::ParseError if the input cannot be parsed.
    regex::Part parse_alternatives() {
        auto part = parse_sequence_or_empty();
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
            alternatives.push_back(parse_sequence_or_empty());
        }
        return regex::part::Alternatives{std::move(alternatives)};
    }

    /// Intermediate rule: parse a sequence of atoms (e.g. `a(?:b)[c-e]`).
    ///
    /// @returns the list of parsed atoms packed into `regex::part::Sequence` or, if and only if
    /// this list of contains exactly 1 element, the only atom unchanged.
    ///
    /// @throws errors::ParseError if the input cannot be parsed.
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

    /// Intermediate rule: parse a possibly empty sequence of atoms.
    ///
    /// @returns `regex::part::Empty` if the sequence is empty, or calls `parse_sequence` otherwise.
    ///
    /// @throws errors::ParseError if the input cannot be parsed.
    regex::Part parse_sequence_or_empty() {
        if (ends_regex(lookahead())) {
            return regex::part::Empty{};
        }
        return parse_sequence();
    }

    /// Intermediate rule: parse an atom.
    ///
    /// Currently, this grammar only recognizes two kinds of atoms: character literals (individual
    /// plain characters in a regex) and parenthesized groups. As the project development goes on,
    /// new kinds of atoms will be added.
    ///
    /// @returns the parsed atom (some variant of `regex::Part` depending on the atom kind).
    ///
    /// @throws errors::ParseError if the input cannot be parsed.
    regex::Part parse_atom() {
        auto la = lookahead();
        if (la == U'(') {
            return parse_group();
        } else {
            return parse_char_literal();
        }
    }

    /// Intermediate rule: parse a character literal.
    ///
    /// @returns the parsed character literal (`regex::part::Literal`).
    ///
    /// @throws errors::UnexpectedEnd if all characters from the input have already been consumed.
    regex::Part parse_char_literal() {
        auto c = next_char();
        if (!c.has_value()) {
            throw errors::UnexpectedEnd(m_pos, "a character");
        }
        return regex::part::Literal{c.value()};
    }

    /// Intermediate rule: parse a parenthesized group (any capture variant).
    ///
    /// @returns the parsed group (`regex::part::Group`).
    regex::Part parse_group() {
        expect_char(U'(', "an opening parenthesis (`(`)");
        auto la = lookahead_nonempty(
            "a closing parenthesis (`)`), a character in a group or a group capture specification");

        // Default-capture (by index) group.
        if (la != U'?') {
            auto inner = parse_regex();
            expect_char(U')', "a closing parenthesis (`)`)");
            return regex::part::Group(regex::capture::Index{}, std::move(inner));
        }
        expect_char(U'?', "a question mark beginning a group capture specification (`?`)");

        constexpr auto expected_msg = "a group capture specification (the part after `?`)";
        la = lookahead_nonempty(expected_msg);

        // Uncaptured group.
        if (la == U':') {
            expect_char(U':', "a colon (`:`)");
            auto inner = parse_regex();
            expect_char(U')', "a closing parenthesis (`)`)");
            return regex::part::Group(regex::capture::None{}, std::move(inner));
        }

        // Group captured by name.

        // `(?P<name>contents)` or `(?<name>contents)` flavors.
        if (la == U'P' || la == U'<') {
            bool has_p = false;
            if (la == U'P') {
                has_p = true;
                expect_char(U'P', "a capture group name marker (`P`)");
            }
            expect_char(U'<', "an opening delimiter for a capture group name (`<`)");
            auto group_name = parse_group_name();
            expect_char(U'>', "a closing delimiter for a capture group name (`>`)");
            auto inner = parse_regex();
            expect_char(U')', "a closing parenthesis (`)`)");
            auto flavor = has_p ? regex::NamedCaptureFlavor::AnglesWithP
                                : regex::NamedCaptureFlavor::Angles;
            return regex::part::Group(
                regex::capture::Name{std::move(group_name), flavor},
                std::move(inner));
        }

        if (la == U'\'') {
            expect_char(U'\'', "an opening delimiter for a capture group name (`'`)");
            auto group_name = parse_group_name();
            expect_char(U'\'', "a closing delimiter for a capture group name (`'`)");
            auto inner = parse_regex();
            expect_char(U')', "a closing parenthesis (`)`)");
            return regex::part::Group(
                regex::capture::Name{std::move(group_name), regex::NamedCaptureFlavor::Apostrophes},
                std::move(inner));
        }

        throw errors::UnexpectedChar(m_pos, la, expected_msg);
    }

    /// Intermediate rule: parse a group name.
    ///
    /// @returns the UTF-8 encoded group name as an `std::string`.
    std::string parse_group_name() {
        constexpr auto first_char_expected_msg = "the first character of a capture group name";
        auto la = lookahead_nonempty(first_char_expected_msg);
        if (!is_valid_for_group_name(la)) {
            throw errors::UnexpectedChar(m_pos, la, first_char_expected_msg);
        }

        std::string group_name;
        push_utf8(group_name, next_char().value());
        while (true) {
            constexpr auto next_char_expected_msg = "a character of a capture group name";
            auto la = lookahead_nonempty(next_char_expected_msg);
            if (!is_valid_for_group_name(la)) {
                break;
            }
            push_utf8(group_name, next_char().value());
        }
        return group_name;
    }

private:
    /// Peek the next character, if any, without consuming it.
    ///
    /// @returns the next character from the input, if any, or `std::nullopt` otherwise.
    std::optional<char32_t> lookahead() {
        if (m_iter == m_end) {
            return std::nullopt;
        }
        return *m_iter;
    }

    /// Peek the next character, without consuming it.
    ///
    /// @throws UnexpectedEnd if the end of input has been reached.
    ///
    /// @returns the next character from the input.
    char32_t lookahead_nonempty(std::string_view expected) {
        auto opt = lookahead();
        if (!opt.has_value()) {
            throw errors::UnexpectedEnd(m_pos, std::string(expected));
        }
        return opt.value();
    }

    /// Peek the next character, if any, and consume it.
    ///
    /// @returns the next character from the input, if any, or `std::nullopt` otherwise.
    std::optional<char32_t> next_char() {
        if (m_iter == m_end) {
            return std::nullopt;
        }
        auto c = *m_iter;
        ++m_iter;
        ++m_pos;
        return c;
    }

    /// Peek the next character and verify that it is the same as expected.
    ///
    /// @throws errors::UnexpectedChar if the expectations are not met.
    /// @throws errors::UnexpectedEnd if the end of input has been reached.
    void expect_char(char32_t expected_char, std::string_view expected_msg) {
        auto c_opt = next_char();
        if (!c_opt.has_value()) {
            // m_pos not changed by `next_char()`.
            throw errors::UnexpectedEnd(m_pos, std::string(expected_msg));
        }
        auto c = c_opt.value();
        if (c != expected_char) {
            // m_pos increased by `next_char()`, subtract 1 to adjust.
            throw errors::UnexpectedChar(m_pos - 1, c, std::string(expected_msg));
        }
    }

    /// Discard several next characters from the input.
    ///
    /// @param num_skipped_chars the number of characters to discard.
    /// @param expected the description of what the characters skipped should look like. It is
    /// incorporated into an exception if the input ends prematurely.
    ///
    /// @throws errors::UnexpectedEnd if the end of input has been reached before all requested
    /// characters have been discarded.
    void advance(size_t num_skipped_chars, std::string_view expected) {
        for (size_t i = 0; i < num_skipped_chars; ++i) {
            if (m_iter == m_end) {
                throw errors::UnexpectedEnd(m_pos, std::string(expected));
            }
            ++m_iter;
            ++m_pos;
        }
    }

    /// Check if an atom can start with a given character.
    ///
    /// Helper function.
    static bool can_start_atom(char32_t c) {
        auto forbidden_chars = std::basic_string_view(U"[](){}|?+*");
        return std::find(forbidden_chars.begin(), forbidden_chars.end(), c)
            == forbidden_chars.end();
    }

    /// Check if an atom can start with a given character.
    ///
    /// Helper function. Takes an std::optional for convenience.
    /// @returns `true` if and only if the provided optional has a value and
    /// `can_start_atom(char32_t)` returns `true` on this value.
    static bool can_start_atom(std::optional<char32_t> c) {
        return c.has_value() && can_start_atom(c.value());
    }

    /// Check if the provided character comes immediately past the end of a regex or a
    /// top-level-like subexpression.
    ///
    /// Helper function. Returns `true` if the provided optional doesn't have a value because the
    /// end of input ends a regex.
    static bool ends_regex(std::optional<char32_t> c) {
        if (!c.has_value()) {
            return true;
        }
        auto chars = std::basic_string_view(U")|");
        return std::find(chars.begin(), chars.end(), c) != chars.end();
    }

    /// Check if the provided character is valid for a capture group name.
    ///
    /// Helper function.
    static bool is_valid_for_group_name(char32_t c) {
        auto forbidden_chars = std::basic_string_view(U"'<>()[]{}|.+?*^$&");
        return std::find(forbidden_chars.begin(), forbidden_chars.end(), c)
            == forbidden_chars.end();
    }

    /// The forward iterator at the current read position.
    Iter m_iter;
    /// The end iterator.
    Sentinel m_end;
    /// The number of characters consumed so far, or, equivalently, the 0-based position in the input.
    size_t m_pos = 0;
};

/// The type deduction guideline for `Parser`.
template <typename Iter, typename Sentinel>
Parser(Iter begin, Sentinel end) -> Parser<Iter, Sentinel>;

regex::Part parse_regex(const utils::UnicodeStringView& regex) {
    auto parser = Parser(regex.begin(), regex.end());
    auto result = parser.parse_regex();
    parser.expect_end();
    return result;
}

}  // namespace wr22::regex_parser::parser
