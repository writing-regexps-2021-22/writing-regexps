#pragma once

// stl
#include <utility>
#include <variant>

namespace wr22::regex_parser::utils {

namespace detail::adt {
    // https://en.cppreference.com/w/cpp/utility/variant/visit#Example provides a very similar
    // example of C++ template black magic.
    template <typename... Fs>
    struct MultiCallable : public Fs... {
        MultiCallable(Fs&&... fs) : Fs(fs)... {}
    };
}  // namespace detail::adt

template <typename... Variants>
class Adt {
public:
    using VariantType = std::variant<Variants...>;

    template <typename V>
    Adt(V variant) : m_variant(std::move(variant)) {}

    template <typename... Fs>
    decltype(auto) visit(Fs&&... visitors) const {
        return std::visit(
            detail::adt::MultiCallable<Fs...>(std::forward<Fs>(visitors)...),
            m_variant);
    }

    template <typename... Fs>
    decltype(auto) visit(Fs&&... visitors) {
        return std::visit(
            detail::adt::MultiCallable<Fs...>(std::forward<Fs>(visitors)...),
            m_variant);
    }

    const VariantType& as_variant() const {
        return m_variant;
    }

    VariantType& as_variant() {
        return m_variant;
    }

protected:
    VariantType m_variant;
};

template <typename... Variants>
bool operator==(const Adt<Variants...>& lhs, const Adt<Variants...>& rhs) {
    return lhs.as_variant() == rhs.as_variant();
}

template <typename... Variants>
bool operator!=(const Adt<Variants...>& lhs, const Adt<Variants...>& rhs) {
    return !(lhs == rhs);
}

}  // namespace wr22::regex_parser::utils
