#pragma once

// wr22
#include <wr22/regex_parser/span/span.hpp>

// stl
#include <functional>

namespace wr22::regex_executor::utils {

template <typename T>
class SpannedRef {
public:
    SpannedRef(const T& item, regex_parser::span::Span span) : m_item(item), m_span(span) {}

    const T& item() const {
        return m_item.get();
    }

    regex_parser::span::Span span() const {
        return m_span;
    }

private:
    std::reference_wrapper<const T> m_item;
    regex_parser::span::Span m_span;
};

}  // namespace wr22::regex_executor::utils
