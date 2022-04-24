// wr22
#include "wr22/regex_parser/span/span.hpp"
#include <wr22/regex_parser/parser/errors.hpp>
#include <wr22/regex_parser/parser/regex.hpp>
#include <wr22/regex_parser/regex/part.hpp>
#include <wr22/unicode/conversion.hpp>

// stl
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace wr22::regex_parser::parser {

using span::Span;

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
    /// @returns the parsed regex AST (some variant of `regex::SpannedPart` depending on the input).
    ///
    /// @throws errors::ParseError if the input cannot be parsed.
    regex::SpannedPart parse_regex() {
        // Empty regexes are handled as empty sequences.
        return parse_alternatives();
    }

    /// Intermediate rule: parse a pipe-separated list of alternatives (e.g. `a|bb|ccc`).
    ///
    /// @returns the list of parsed alternatives packed into `regex::part::Alternatives` or, if and
    /// only if the list of alternatives contains exactly 1 element, the only alternative unchanged.
    ///
    /// @throws errors::ParseError if the input cannot be parsed.
    regex::SpannedPart parse_alternatives() {
        auto begin = track_pos();

        auto part = parse_sequence_or_empty();
        // Either this is the only alternative (in which case we flatten the AST and return the only
        // alternative itself)...
        if (lookahead() != U'|') {
            return part;
        }

        // ... or we will be constructing a vector of alternatives.
        // Note: we cannot use an initializer list, because this would
        // entail copying `part`, whose copy constructor is deleted.
        std::vector<regex::SpannedPart> alternatives;
        alternatives.push_back(std::move(part));
        while (lookahead() == U'|') {
            advance(1, "`|`");
            alternatives.push_back(parse_sequence_or_empty());
        }
        return make_spanned(begin, regex::part::Alternatives(std::move(alternatives)));
    }

    /// Intermediate rule: parse a sequence of atoms (e.g. `a(?:b)[c-e]`).
    ///
    /// @returns the list of parsed atoms packed into `regex::part::Sequence` or, if and only if
    /// this list of contains exactly 1 element, the only atom unchanged.
    ///
    /// @throws errors::ParseError if the input cannot be parsed.
    regex::SpannedPart parse_sequence() {
        auto begin = track_pos();

        auto part = parse_atom();
        if (!can_start_atom(lookahead())) {
            return part;
        }

        std::vector<regex::SpannedPart> items;
        items.push_back(std::move(part));
        while (can_start_atom(lookahead())) {
            items.push_back(parse_atom());
        }
        return make_spanned(begin, regex::part::Sequence(std::move(items)));
    }

    /// Intermediate rule: parse a possibly empty sequence of atoms.
    ///
    /// @returns `regex::part::Empty` if the sequence is empty, or calls `parse_sequence` otherwise.
    ///
    /// @throws errors::ParseError if the input cannot be parsed.
    regex::SpannedPart parse_sequence_or_empty() {
        auto begin = track_pos();
        if (ends_regex(lookahead())) {
            return make_spanned(begin, regex::part::Empty());
        }
        return parse_sequence();
    }

    /// Intermediate rule: parse an atom.
    ///
    /// Currently, this grammar only recognizes two kinds of atoms: character literals (individual
    /// plain characters in a regex) and parenthesized groups. As the project development goes on,
    /// new kinds of atoms will be added.
    ///
    /// @returns the parsed atom (some variant of `regex::SpannedPart` depending on the atom kind).
    ///
    /// @throws errors::ParseError if the input cannot be parsed.
    regex::SpannedPart parse_atom() {
        auto begin = track_pos();
        std::optional<regex::SpannedPart> result;
        auto la1 = lookahead_nonempty("an openning parenthesis (`(`) or a plain character");
        if (la1 == U'(') {
            result = parse_group();
        } else if (la1 == U'.') {
            result = parse_wildcard();
        } else if (la1 == U'[') {
            result = parse_char_class();
        } else {
            result = parse_char_literal();
        }

        // Here come quantifiers.

        auto la2 = lookahead();
        if (la2 == U'?') {
            expect_char(U'?', "a question mark denoting an optional quantifier (`?`)");
            return make_spanned(begin, regex::part::Optional(std::move(result.value())));
        }
        if (la2 == U'*') {
            expect_char(U'*', "an asterisk denoting an \"at least zero\" quantifier (`*`)");
            return make_spanned(begin, regex::part::Star(std::move(result.value())));
        }
        if (la2 == U'+') {
            expect_char(U'+', "a plus sign denoting an \"at least one\" quantifier (`+`)");
            return make_spanned(begin, regex::part::Plus(std::move(result.value())));
        }
        return std::move(result.value());
    }

    /// Intermediate rule: parse a wildcard (`.`).
    ///
    /// @returns the wildcard AST node.
    ///
    /// @throws errors::UnexpectedEnd if all characters from the input have already been consumed.
    /// @throws errors::UnexpectedChar if the next input character is not `.`.
    regex::SpannedPart parse_wildcard() {
        auto position = m_pos;
        expect_char(U'.', "the wildcard character (`.`)");
        return regex::SpannedPart(regex::part::Wildcard(), Span::make_single_position(position));
    }

    /// Intermediate rule: parse a character literal.
    ///
    /// @returns the parsed character literal (`regex::part::Literal`).
    ///
    /// @throws errors::UnexpectedEnd if all characters from the input have already been consumed.
    regex::SpannedPart parse_char_literal() {
        auto position = m_pos;
        auto c = next_char_validated(is_valid_for_char_literal, "a plain character");
        return regex::SpannedPart(regex::part::Literal(c), Span::make_single_position(position));
    }

    /// Intermediate rule: parse a parenthesized group (any capture variant).
    ///
    /// @returns the parsed group (`regex::part::Group`).
    regex::SpannedPart parse_group() {
        auto begin = track_pos();

        expect_char(U'(', "an opening parenthesis (`(`)");
        auto la = lookahead_nonempty(
            "a closing parenthesis (`)`), a character in a group or a group capture specification");

        // Default-capture (by index) group.
        if (la != U'?') {
            auto inner = parse_regex();
            expect_char(U')', "a closing parenthesis (`)`)");
            return make_spanned(
                begin,
                regex::part::Group(regex::capture::Index(), std::move(inner)));
        }
        expect_char(U'?', "a question mark beginning a group capture specification (`?`)");

        constexpr auto expected_msg = "a group capture specification (the part after `?`)";
        la = lookahead_nonempty(expected_msg);

        // Uncaptured group.
        if (la == U':') {
            expect_char(U':', "a colon (`:`)");
            auto inner = parse_regex();
            expect_char(U')', "a closing parenthesis (`)`)");
            return make_spanned(begin, regex::part::Group(regex::capture::None(), std::move(inner)));
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
            auto&& [group_name, group_name_span] = parse_group_name();
            expect_char(U'>', "a closing delimiter for a capture group name (`>`)");
            auto inner = parse_regex();
            expect_char(U')', "a closing parenthesis (`)`)");
            auto flavor = has_p ? regex::NamedCaptureFlavor::AnglesWithP
                                : regex::NamedCaptureFlavor::Angles;
            return make_spanned(
                begin,
                regex::part::Group(
                    regex::capture::Name(std::move(group_name), flavor),
                    std::move(inner)));
        }

        // `(?'name'contents)` flavor.
        if (la == U'\'') {
            expect_char(U'\'', "an opening delimiter for a capture group name (`'`)");
            auto&& [group_name, group_name_span] = parse_group_name();
            expect_char(U'\'', "a closing delimiter for a capture group name (`'`)");
            auto inner = parse_regex();
            expect_char(U')', "a closing parenthesis (`)`)");
            return make_spanned(
                begin,
                regex::part::Group(
                    regex::capture::Name(
                        std::move(group_name),
                        regex::NamedCaptureFlavor::Apostrophes),
                    std::move(inner)));
        }

        throw errors::UnexpectedChar(m_pos, la, expected_msg);
    }

    /// Intermediate rule: parse a group name.
    ///
    /// @returns the UTF-8 encoded group name as an `std::string`.
    std::pair<std::string, Span> parse_group_name() {
        auto begin_pos = m_pos;

        constexpr auto first_char_expected_msg = "the first character of a capture group name";
        auto la = lookahead_nonempty(first_char_expected_msg);
        if (!is_valid_for_group_name(la)) {
            throw errors::UnexpectedChar(m_pos, la, first_char_expected_msg);
        }

        std::string group_name;
        wr22::unicode::to_utf8_append(group_name, next_char().value());
        while (true) {
            constexpr auto next_char_expected_msg = "a character of a capture group name";
            auto la = lookahead_nonempty(next_char_expected_msg);
            if (!is_valid_for_group_name(la)) {
                break;
            }
            wr22::unicode::to_utf8_append(group_name, next_char().value());
        }

        auto end_pos = m_pos;
        return std::make_pair(std::move(group_name), Span::make_from_positions(begin_pos, end_pos));
    }

    /// Intermediate rule: parse a character class (e.g. [^a-z0-9_-]).
    ///
    /// @returns the character class AST node.
    regex::SpannedPart parse_char_class() {
        using regex::CharacterRange;
        using regex::SpannedCharacterRange;
        using span::Span;

        auto begin = track_pos();
        expect_char(U'[', "an opening bracket");

        bool inverted = false;
        std::vector<SpannedCharacterRange> ranges;

        enum class State {
            Initial,
            Inverted,
            Normal,
            MidRange,
        };
        auto state = State::Initial;

        // Invariant: `current_char` has value if and only if `current_span` does.
        // This invariant is upheld by always either assigning to both variables, or
        // re-assigning a value to either of the variables if it already has a value.
        std::optional<char32_t> current_char = std::nullopt;
        std::optional<Span> current_span = std::nullopt;

        try {
            while (true) {
                // Read the next character.
                auto c = next_char_nonempty(
                    "a character, a character range, or a closing bracket (']')");

                // State transitions.
                if (c == U'^' && state == State::Initial) {
                    // The caret character indicating that the match should be inverted.
                    inverted = true;
                    state = State::Inverted;
                } else if (c == U']') {
                    if (state == State::Initial || state == State::Inverted) {
                        // ']' as the first character is just a normal character.
                        current_char = c;
                        current_span = Span::make_single_position(m_pos - 1);
                        state = State::Normal;
                    } else if (state == State::MidRange) {
                        // The sequence of `X-]` has occurred for some `X`. This is not
                        // a range but instead two single characters (`X` and `-`) followed by the
                        // character class termination.
                        //
                        // `current_char` and `current_span` must have values if the state is
                        // MidRange.
                        auto current_span_value = current_span.value();

                        // `current_span` must cover the two characters `X` and `-`.
                        assert(current_span_value.length() == 2);

                        // Covers only `X`.
                        auto first_span = Span::make_single_position(current_span_value.begin());
                        // Covers only '-'.
                        auto second_span = Span::make_single_position(
                            current_span_value.begin() + 1);

                        // Add the ranges to the list.
                        ranges.push_back(SpannedCharacterRange{
                            .range = CharacterRange::from_single_character(current_char.value()),
                            .span = first_span,
                        });
                        ranges.push_back(SpannedCharacterRange{
                            .range = CharacterRange::from_single_character(U'-'),
                            .span = second_span,
                        });

                        current_char = std::nullopt;
                        current_span = std::nullopt;
                        state = State::Normal;
                        // The character class has terminated.
                        break;
                    } else {
                        // Character class termination (simple case).
                        if (current_char.has_value()) {
                            // If we have a character we have not yet added to the range list, fix
                            // this.
                            ranges.push_back(SpannedCharacterRange{
                                .range = CharacterRange::from_single_character(current_char.value()),
                                .span = current_span.value(),
                            });
                        }

                        current_char = std::nullopt;
                        current_span = std::nullopt;
                        state = State::Normal;
                        // The character class has terminated.
                        break;
                    }
                } else if (c == U'-') {
                    if (state == State::MidRange) {
                        // A range `X--` for some `X`, where we have just read the second `-`.
                        // `current_char` and `current_span` must have values if the state is
                        // MidRange.

                        // The current span is extended to account for the just read character.
                        ranges.push_back(SpannedCharacterRange{
                            .range = CharacterRange::from_endpoints(current_char.value(), c),
                            .span = current_span.value().extend_right(1),
                        });

                        current_char = std::nullopt;
                        current_span = std::nullopt;
                        state = State::Normal;
                    } else if (current_char.has_value()) {
                        // The sequence `X-` for some `X`, where we have just read the `-`.
                        // In most cases, this is the beginning of the character range, but
                        // it will be determined later if it is the case.
                        //
                        // Due to the `current_char`/`current_span` relation invariant,
                        // `current_span` must have a value.
                        state = State::MidRange;

                        // Extend the current span.
                        current_span = current_span.value().extend_right(1);
                    } else {
                        /// `-` is found right after the character range started or right after
                        /// another range. In either case, it is considered as a plain character.
                        current_char = c;
                        current_span = Span::make_single_position(m_pos - 1);
                        state = State::Normal;
                    }
                } else {
                    if (state == State::MidRange) {
                        // A range `X-Y` for some `X` and `Y` where we have just read `Y`.
                        // `current_char` and `current_span` must have values if the state is
                        // MidRange.

                        // The current span is extended to account for the just read character.
                        ranges.push_back(SpannedCharacterRange{
                            .range = CharacterRange::from_endpoints(current_char.value(), c),
                            .span = current_span.value().extend_right(1),
                        });
                        current_char = std::nullopt;
                        current_span = std::nullopt;
                    } else {
                        // Some plain character `Y` which is not a right bound of a range.
                        if (current_char.has_value()) {
                            // This character follows another plain character `X` (for some `X`).
                            // Add `X` to the list of ranges.
                            ranges.push_back(SpannedCharacterRange{
                                .range = CharacterRange::from_single_character(current_char.value()),
                                .span = current_span.value(),
                            });
                        }
                        current_char = c;
                        current_span = Span::make_single_position(m_pos - 1);
                    }
                    state = State::Normal;
                }
            }
        } catch (const regex::InvalidCharacterRange& e) {
            // `current_span` must have a value whenever a character range is constructed,
            // which means it must do so here.
            throw errors::InvalidRange(current_span.value(), e.first, e.last);
        }

        return make_spanned(
            begin,
            regex::part::CharacterClass(regex::CharacterClassData{
                .ranges = std::move(ranges),
                .inverted = inverted,
            }));
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

    /// Peek the next character, and consume it.
    ///
    /// @returns the next character from the input.
    ///
    /// @throws UnexpectedEnd if the end of input has been reached.
    char32_t next_char_nonempty(std::string_view expected) {
        return next_char_validated([]([[maybe_unused]] auto c) { return true; }, expected);
    }

    /// Peek the next character, validate it and consume it.
    ///
    /// @param predicate a function that takes the character from the input and returns `true` if it
    /// is valid (expected) or `false` otherwise.
    /// @param expected_msg a description of what kind of characters are expected.
    ///
    /// @throws UnexpectedChar if the validation fails.
    /// @throws UnexpectedEnd if the end of input has been reached.
    ///
    /// @returns the next character from the input.
    template <typename F>
    char32_t next_char_validated(const F& predicate, std::string_view expected_msg) {
        auto c_opt = next_char();
        if (!c_opt.has_value()) {
            // m_pos not changed by `next_char()`.
            throw errors::UnexpectedEnd(m_pos, std::string(expected_msg));
        }
        auto c = c_opt.value();
        if (!predicate(c)) {
            // m_pos increased by `next_char()`, subtract 1 to adjust.
            throw errors::UnexpectedChar(m_pos - 1, c, std::string(expected_msg));
        }
        return c;
    }

    /// Peek the next character and verify that it is the same as expected.
    ///
    /// @throws errors::UnexpectedChar if the expectations are not met.
    /// @throws errors::UnexpectedEnd if the end of input has been reached.
    void expect_char(char32_t expected_char, std::string_view expected_msg) {
        next_char_validated([expected_char](auto c) { return c == expected_char; }, expected_msg);
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
        return c == '(' || c == '.' || c == '[' || is_valid_for_char_literal(c);
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

    /// Check if the provided character is valid for a plain character literal.
    ///
    /// Helper function.
    static bool is_valid_for_char_literal(char32_t c) {
        auto forbidden_chars = std::basic_string_view(U"()[{|.+?*^$&\\");
        return std::find(forbidden_chars.begin(), forbidden_chars.end(), c)
            == forbidden_chars.end();
    }

    /// A newtype wrapper around the input position (point, not range).
    struct PositionTracker {
        size_t position;
    };

    /// Remember the `begin` position to construct a range position later.
    ///
    /// Helper method.
    PositionTracker track_pos() const {
        return PositionTracker{m_pos};
    }

    /// Wrap a `Part` into a `SpannedPart` based on the information remembered.
    ///
    /// Helper method.
    regex::SpannedPart make_spanned(PositionTracker position_tracker, regex::Part part) const {
        auto span = Span::make_from_positions(position_tracker.position, m_pos);
        return regex::SpannedPart(std::move(part), span);
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

regex::SpannedPart parse_regex(const std::u32string_view& regex) {
    auto parser = Parser(regex.begin(), regex.end());
    auto result = parser.parse_regex();
    parser.expect_end();
    return result;
}

}  // namespace wr22::regex_parser::parser
