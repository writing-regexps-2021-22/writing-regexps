// wr22
#include <boost/locale/encoding_errors.hpp>
#include <boost/locale/utf.hpp>
#include <wr22/unicode/conversion.hpp>

// stl
#include <iterator>

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
    for (auto char_utf32 : string_utf32) {
        to_utf8_append(buffer, char_utf32);
    }
}

void to_utf8_append(std::string& buffer, char32_t char_utf32) {
    auto codepoint = static_cast<boost::locale::utf::code_point>(char_utf32);
    boost::locale::utf::utf_traits<char>::encode(codepoint, std::back_inserter(buffer));
}

std::u32string from_utf8(const std::string_view& string_utf8) {
    return boost::locale::conv::utf_to_utf<char32_t>(string_utf8.begin(), string_utf8.end());
}

void from_utf8_append(std::u32string& buffer, const std::string_view& string_utf8) {
    auto it = string_utf8.begin();
    auto end = string_utf8.end();
    while (it != end) {
        auto codepoint = boost::locale::utf::utf_traits<char>::decode(it, string_utf8.end());
        if (!boost::locale::utf::is_valid_codepoint(codepoint)) {
            throw boost::locale::conv::conversion_error{};
        }
        auto char_utf32 = static_cast<char32_t>(codepoint);
        buffer.push_back(char_utf32);
    }

}

}  // namespace wr22::unicode
