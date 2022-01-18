#pragma once

// stl
#include <utility>
#include <variant>

namespace wr22::regex_parser::utils {

namespace detail::adt {
    // https://en.cppreference.com/w/cpp/utility/variant/visit#Example provides a very similar
    // example of C++ template black magic.
    template <typename... Fs>
    struct MultiCallable : public Fs... {};
}  // namespace detail::adt

template <typename... Variants>
class Adt {
public:
    template <typename V>
    Adt(V variant) : m_variant(std::move(variant)) {}

    template <typename... Fs>
    decltype(auto) visit(Fs&&... visitors) const {
        return std::visit(MultiCallable<Fs...>(std::forward<Fs>(visitors)...), m_variant);
    }

    template <typename... Fs>
    decltype(auto) visit(Fs&&... visitors) {
        return std::visit(MultiCallable<Fs...>(std::forward<Fs>(visitors)...), m_variant);
    }

    bool operator==(const Adt<Variants...>& rhs) const {
        return m_variant == rhs.m_variant;
    }

    bool operator!=(const Adt<Variants...>& rhs) const {
        return !(*this == rhs);
    }

protected:
    using VariantType = std::variant<Variants...>;
    VariantType m_variant;
};

}  // namespace wr22::regex_parser::utils
