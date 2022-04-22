// wr22
#include <wr22/regex_parser/regex/character_range.hpp>
#include <wr22/unicode/conversion.hpp>

namespace wr22::regex_parser::regex {

InvalidCharacterRange::InvalidCharacterRange(char32_t first, char32_t last)
    : std::runtime_error(fmt::format(
        "Invalid or empty character range: first = (char32_t){} must be no greater than last = "
        "(char32_t){}",
        wr22::unicode::to_utf8(first),
        wr22::unicode::to_utf8(last))) {}

CharacterRange CharacterRange::from_endpoints(char32_t first, char32_t last) {
    return CharacterRange(first, last);
}

CharacterRange CharacterRange::from_single_character(char32_t character) noexcept {
    return CharacterRange(character, character);
}

char32_t CharacterRange::first() const noexcept {
    return m_first;
}

char32_t CharacterRange::last() const noexcept {
    return m_last;
}

bool CharacterRange::contains(char32_t character) const noexcept {
    return m_first <= character && character <= m_last;
}

bool CharacterRange::is_single_character() const noexcept {
    return m_first == m_last;
}

CharacterRange::CharacterRange(char32_t first, char32_t last) : m_first(first), m_last(last) {
    if (m_last < m_first) {
        throw InvalidCharacterRange(m_first, m_last);
    }
}

void to_json(nlohmann::json& j, const CharacterRange& range) {
    j = nlohmann::json::object();
    auto single_char = range.is_single_character();
    j["single_char"] = single_char;

    if (single_char) {
        j["char"] = wr22::unicode::to_utf8(range.first());
    } else {
        j["first_char"] = wr22::unicode::to_utf8(range.first());
        j["last_char"] = wr22::unicode::to_utf8(range.last());
    }
}

}  // namespace wr22::regex_parser::regex
