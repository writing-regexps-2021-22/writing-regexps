#pragma once

// STL
#include <compare>
#include <cstddef>
#include <exception>
#include <optional>
#include <string_view>

namespace wr22::regex_parser::utils {

class UnicodeStringViewIterator;

/// A wrapper around `std::string_view` that enables UTF-8 codepoint iteration.
///
/// An `UnicodeStringView` holds a "raw" `std::string_view` and assumes it is
/// UTF-8 encoded. It provides the `begin()` & `end()` methods, which return iterators
/// that work with Unicode codepoints instead of raw bytes. This makes this type useful
/// in contexts where one needs to iterate over Unicode code points (like characters,
/// but technically a different thing) in an `std::string` or an `std::string_view`.
class UnicodeStringView {
    friend class UnicodeStringViewIterator;

public:
    /// An error thrown when the string's encoding is not valid UTF-8.
    struct InvalidUtf8 : std::exception {
        const char* what() const noexcept override;
    };

    /// Constructor. Wrap an existing `std::string_view`.
    ///
    /// The ownership of the data pointed to by `raw` is not taken, and the string contents
    /// are not copied. Hence, just like with an ordinary `std::string_view`, if the pointed
    /// data is invalidated, the `UnicodeStringView` referring to it is invalidated as well.
    ///
    /// This constructor does not check that `raw` is a correct UTF-8 encoded string.
    ///
    /// SAFETY:
    /// If the string contents change during the `UnicodeStringView`'s lifetime or the string is
    /// destroyed, the behavior is undefined.
    explicit UnicodeStringView(const std::string_view& raw);

    /// Access the wrapped `std::string_view`.
    const std::string_view& raw() const;

    /// Create an iterator to the beginning of the string.
    ///
    /// SAFETY:
    /// The returned iterator must not outlive this object.
    UnicodeStringViewIterator begin() const;

    /// Create an iterator past the end of the string.
    ///
    /// SAFETY:
    /// The returned iterator must not outlive this object.
    UnicodeStringViewIterator end() const;

private:
    std::string_view m_raw;
};

/// Iterator over code points for `UnicodeStringView`.
///
/// SAFETY:
/// No object of this class must outlive the `UnicodeStringView` that has created it.
/// If this is violated, the behavior is undefined.
class UnicodeStringViewIterator {
    friend class UnicodeStringView;

public:
    /// The default constructor is meaningless and is thus deleted.
    UnicodeStringViewIterator() = delete;

    /// Iterator interface: pre-increment.
    ///
    /// @throws UnicodeStringView::InvalidUtf8 if the codepoint decodes into an invalid or
    /// incomplete value.
    UnicodeStringViewIterator& operator++();

    /// Iterator interface: dereferencing.
    ///
    /// @throws UnicodeStringView::InvalidUtf8 if the codepoint decodes into an invalid or
    /// incomplete value.
    char32_t operator*();

    std::strong_ordering operator<=>(const UnicodeStringViewIterator& rhs) const;

private:
    using UnderlyingIterator = std::string_view::const_iterator;

    explicit UnicodeStringViewIterator(
        const UnderlyingIterator& iter,
        const UnderlyingIterator& end);
    std::optional<char32_t> decode_current_codepoint();
    void update_current_codepoint_size();

    UnderlyingIterator m_iter;
    UnderlyingIterator m_end;

    // A memoization of the result of decoding of the last code point.
    // This is done purely for optimization to avoid having to decode a code point
    // twice: for dereferencing the iterator and for incrementing it. The value
    // of 0 indicates an absence of a memoized value (0 is not a valid byte size of
    // a codepoint).
    size_t m_last_codepoint_size = 0;
};

}  // namespace wr22::regex_parser::utils
