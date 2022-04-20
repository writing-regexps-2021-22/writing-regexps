// wr22
#include <wr22/regex_parser/regex/character_range.hpp>

namespace wr22::regex_parser::regex {

explicit InvalidCharacterRange::InvalidCharacterRange(char32_t first, char32_t last)
    : std::runtime_error(fmt::format(
        "Invalid or empty character range: first = (char32_t){} must be no greater than last = "
        "(char32_t){}",
        first,
        last)) {}

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

CharacterRange::CharacterRange(char32_t first, char32_t last) : m_first(first), m_last(last) {
    if (m_last < m_first) {
        throw InvalidCharacterRange(m_first, m_end);
    }
}

}  // namespace wr22::regex_parser::regex
