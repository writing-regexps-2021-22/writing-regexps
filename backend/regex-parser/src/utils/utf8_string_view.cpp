// wr22
#include <wr22/regex_parser/utils/utf8_string_view.hpp>

// boost
#include <boost/locale/utf.hpp>

// STL
#include <cassert>

namespace wr22::regex_parser::utils {

using utf_traits = boost::locale::utf::utf_traits<char>;

const char* UnicodeStringView::InvalidUtf8::what() const noexcept {
    return "The string is not UTF-8 encoded, which is a required precondition for an "
           "UnicodeStringView";
}

UnicodeStringView::UnicodeStringView(const std::string_view& raw) : m_raw(raw) {}

const std::string_view& UnicodeStringView::raw() const {
    return m_raw;
}

UnicodeStringViewIterator UnicodeStringView::begin() const {
    return UnicodeStringViewIterator(m_raw.begin(), m_raw.end());
}

UnicodeStringViewIterator UnicodeStringView::end() const {
    return UnicodeStringViewIterator(m_raw.end(), m_raw.end());
}

UnicodeStringViewIterator::UnicodeStringViewIterator(
    const UnderlyingIterator& iter,
    const UnderlyingIterator& end)
    : m_iter(iter), m_end(end) {}

UnicodeStringViewIterator& UnicodeStringViewIterator::operator++() {
    update_current_codepoint_size();
    m_iter += m_last_codepoint_size;
    m_last_codepoint_size = 0;
    return *this;
}

char32_t UnicodeStringViewIterator::operator*() {
    auto codepoint_opt = decode_current_codepoint();
    if (!codepoint_opt.has_value()) {
        throw UnicodeStringView::InvalidUtf8{};
    }

    auto codepoint = codepoint_opt.value();
    return codepoint_opt.value();
}

std::strong_ordering UnicodeStringViewIterator::operator<=>(
    const UnicodeStringViewIterator& rhs) const {
    return m_iter <=> rhs.m_iter;
}

std::optional<char32_t> UnicodeStringViewIterator::decode_current_codepoint() {
    auto boost_codepoint = utf_traits::decode(m_iter, m_end);
    if (!boost::locale::utf::is_valid_codepoint(boost_codepoint)) {
        return std::nullopt;
    }
    m_last_codepoint_size = static_cast<size_t>(utf_traits::width(boost_codepoint));
    return static_cast<char32_t>(boost_codepoint);
}

void UnicodeStringViewIterator::update_current_codepoint_size() {
    if (m_last_codepoint_size == 0) {
        decode_current_codepoint();
    }
    assert(m_last_codepoint_size != 0);
}

}  // namespace wr22::regex_parser::utils
