# unicode
A small library that simplifies conversions between `std::u32string(_view)` and `std::string(_view)`.

`std::string`s are always assumed to be UTF-8 encoded. Thus the function `from_utf8` creates
an `std::u32string` **from** an `std::string` (or an `std::string_view`),
and the function `to_utf8` converts an `std::u32string`, `std::u32string_view` or `char32_t` **to**
an `std::string`.
