#pragma once

// stl
#include <string>
#include <string_view>

namespace wr22::unicode {

/// Convert a `u32string` or a `u32string_view` to a `string`.
///
/// Usage example:
/// ```cpp
/// std::u32string s1 = U"Тест";
/// std::string s2 = to_utf8(s1);
/// std::cout << s2 << std::endl; // Prints "Тест"
/// ```
std::string to_utf8(const std::u32string_view& string_utf32);

/// Convert a `char32_t` to a `string`.
/// The resulting string encodes this character in UTF-8.
/// A `string` is returned and not a `char`, because one `char32_t`
/// can be encoded by multiple `char`s.
///
/// Usage example:
/// ```cpp
/// char32_t c1 = U'λ';
/// std::string s2 = to_utf8(c1);
/// std::cout << s2 << std::endl; // Prints "λ"
/// ```
std::string to_utf8(char32_t char_utf32);

/// Convert a `u32string` or a `u32string_view` to a `string`.
/// Equivalent to `to_utf8`, but appends the resulting string
/// to an existing one instead of returning a new string, which
/// is slightly more efficient than `buffer += to_utf8(string_utf32)`.
///
/// If not sure which function to choose, start with `to_utf8` instead
/// of `to_utf8_append`.
///
/// Usage example:
/// ```cpp
/// std::string sentence = "Hello in Russian is ";
/// std::u32string rus_hello = U"Привет";
/// to_utf8_append(sentence, rus_hello);
/// std::cout << sentence << std::endl; // Prints "Hello in Russian is Привет"
/// ```
void to_utf8_append(std::string& buffer, const std::u32string_view& string_utf32);

/// Convert a `char32_t` to a `string`.
/// The resulting string encodes this character in UTF-8.
/// Equivalent to `to_utf8`, but appends the resulting string
/// to an existing one instead of returning a new string, which
/// is slightly more efficient than `buffer += to_utf8(char_utf32)`.
///
/// If not sure which function to choose, start with `to_utf8` instead
/// of `to_utf8_append`.
///
/// Usage example:
/// ```cpp
/// std::string formula = "2 ";
/// char32_t times = U'×';
/// to_utf8_append(formula, times);
/// formula += " 2 = 4";
/// std::cout << formula << std::endl; // Prints "2 × 2 = 4"
/// ```
void to_utf8_append(std::string& buffer, char32_t char_utf32);

/// Convert a `string` or a `string_view` to a `u32string`.
///
/// @throws `boost::locale::utf::conversion_error` if `string_utf8`
/// contains invalid UTF-8.
///
/// Usage example:
/// ```cpp
/// std::string s1 = "Hello world!";
/// std::u32string s2 = from_utf8(s1);
/// assert(s2 == U"Hello world!");
/// ```
std::u32string from_utf8(const std::string_view& string_utf8);

/// Convert a `string` or a `string_view` to a `u32string`.
/// Equivalent to `from_utf8`, but appends the resulting string
/// to an existing one instead of returning a new string, which
/// is slightly more efficient than `buffer += from_utf8(string_utf8)`.
///
/// If not sure which function to choose, start with `from_utf8` instead
/// of `from_utf8_append`.
///
/// @throws `boost::locale::utf::conversion_error` if `string_utf8`
/// contains invalid UTF-8.
///
/// Usage example:
/// ```cpp
/// std::u32string report = U"Pressure sensor reading is ";
/// std::string reading = "5 atm";
/// from_utf8_append(report, reading);
/// assert(report == U"Pressure sensor reading is 5 atm");
/// ```
void from_utf8_append(std::u32string& buffer, const std::string_view& string_utf8);

}  // namespace wr22::unicode
