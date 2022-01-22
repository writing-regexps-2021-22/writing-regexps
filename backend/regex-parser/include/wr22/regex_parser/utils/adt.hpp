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

/// A helper class that simplifies creation of algebraic data types.
///
/// Algebraic data types are data types that can have one type of a predefined set of variants, but
/// be stored and represented as values of one common type. In C++,
/// [`std::variant`](https://en.cppreference.com/w/cpp/utility/variant) serves exactly this purpose.
/// It is, however, not very convenient to work with or build upon, so this class is designed to
/// simplify building new algebraic data types. It still uses `std::variant` under the hood.
///
/// The template type parameters are the types that the variants may hold (must be distinct types).
template <typename... Variants>
class Adt {
public:
    /// A convenience type alias for the concrete `std::variant` type used.
    using VariantType = std::variant<Variants...>;

    /// Constructor for each of the variants.
    ///
    /// Construct an instance holding a specified variant. The type `V` of the variant provided
    /// must be one of the types from `Variants`. Note that this constructor is purposefully
    /// implicit, so that the variants as separate types are transparently converted to this common
    /// type when necessary.
    ///
    /// The variant is taken by value and moved thereafter, so that, when constructing the common
    /// type, the variant may be either copied or moved, depending on the user's intentions.
    template <typename V>
    Adt(V variant) : m_variant(std::move(variant)) {}

    /// Visit the ADT, applying the suitable function from the list of visitors on the variant held.
    ///
    /// Using this method is essentially the same as using
    /// [`std::visit`](https://en.cppreference.com/w/cpp/utility/variant/visit) on the variant,
    /// except that, for convenience, multiple visitors are joined into one big visitor. That is, a
    /// typical `Adt` usage might look like this:
    ///
    /// ```cpp
    /// struct MyAdt : public Adt<int, double> {
    ///     // Make the constructor available in the derived class.
    ///     using Adt<int, double>::Adt;
    /// };
    ///
    /// // <...>
    ///
    /// void func() {
    ///     // Variant type: double.
    ///     MyAdt my_adt = 3.14;
    ///     // Prints "Double: 3.14".
    ///     my_adt.visit(
    ///         [](int x)    { std::cout << "Int: "    << x << std::endl; },
    ///         [](double x) { std::cout << "Double: " << x << std::endl; }
    ///     );
    /// }
    /// ```
    ///
    /// This is the constant version of the method. Visitors must be callable with the const
    /// reference to variant types.
    template <typename... Fs>
    decltype(auto) visit(Fs&&... visitors) const {
        return std::visit(
            detail::adt::MultiCallable<Fs...>(std::forward<Fs>(visitors)...),
            m_variant);
    }

    /// Visit the ADT, applying the suitable function from the list of visitors on the variant held.
    ///
    /// This is the non-constant version of the method. See the docs for the constant version for a
    /// detailed description and code examples. The only thing different in this version of the
    /// method is that the visitors get called with a non-const lvalue reference to the variants
    /// instead of a const reference.
    template <typename... Fs>
    decltype(auto) visit(Fs&&... visitors) {
        return std::visit(
            detail::adt::MultiCallable<Fs...>(std::forward<Fs>(visitors)...),
            m_variant);
    }

    /// Access the underlying `std::variant` type (constant version).
    const VariantType& as_variant() const {
        return m_variant;
    }

    /// Access the underlying `std::variant` type (non-constant version).
    VariantType& as_variant() {
        return m_variant;
    }

protected:
    VariantType m_variant;
};

/// Compare two compatible ADTs for equality.
template <typename... Variants>
bool operator==(const Adt<Variants...>& lhs, const Adt<Variants...>& rhs) {
    return lhs.as_variant() == rhs.as_variant();
}

/// Compare two compatible ADTs for non-equality.
template <typename... Variants>
bool operator!=(const Adt<Variants...>& lhs, const Adt<Variants...>& rhs) {
    return !(lhs == rhs);
}

}  // namespace wr22::regex_parser::utils
