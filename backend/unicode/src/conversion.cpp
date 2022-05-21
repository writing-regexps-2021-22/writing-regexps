// wr22
#include <wr22/unicode/conversion.hpp>

// stl
#include <iterator>
#include <string>

// boost
#include <boost/locale/encoding_utf.hpp>

namespace wr22::unicode {

std::string to_utf8(const std::u32string_view& string_utf32) {
    return boost::locale::conv::utf_to_utf<char>(string_utf32.begin(), string_utf32.end());
}

std::string to_utf8(char32_t char_utf32) {
    auto begin = &char_utf32;
    auto end = begin + 1;
    return boost::locale::conv::utf_to_utf<char>(begin, end);
}

void to_utf8_append(std::string& buffer, const std::u32string_view& string_utf32) {
    auto output = std::back_inserter(buffer);
    to_utf8_write(output, string_utf32);
}

void to_utf8_append(std::string& buffer, char32_t char_utf32) {
    auto output = std::back_inserter(buffer);
    to_utf8_write(output, char_utf32);
}

std::u32string from_utf8(const std::string_view& string_utf8) {
    return boost::locale::conv::utf_to_utf<char32_t>(string_utf8.begin(), string_utf8.end());
}

void from_utf8_append(std::u32string& buffer, const std::string_view& string_utf8) {
    auto output = std::back_inserter(buffer);
    from_utf8_write(output, string_utf8);
}

}  // namespace wr22::unicode
