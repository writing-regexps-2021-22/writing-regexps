#pragma once

// stl
#include <exception>
#include <memory>
#include <utility>

namespace wr22::regex_parser::utils {

struct BoxIsEmpty : public std::exception {
    const char* what() const noexcept override;
};

/// A copyable and equality-comparable wrapper around `std::unique_ptr`.
///
/// The behavior of this wrapper regarding copying and equality comparison are akin to that of
/// Rust's [`std::boxed::Box`](https://doc.rust-lang.org/std/boxed/struct.Box.html), and hence the
/// class's name. Namely, when testing for (in)equality, the wrapped values are compared instead of
/// raw pointers, and, when wrapped values are copyable, copying a `Box` creates another
/// `std::unique_ptr` with a copy of the wrapped value.
///
/// A `Box` usually contains a value. However, it may become empty when it is moved from. To ensure
/// safety, most operations on an empty box will throw a `BoxIsEmpty` exception instead of causing
/// undefined bahavior.
template <typename T>
class Box {
public:
    /// Constructor that places a value inside the wrapped `std::unique_ptr`.
    ///
    /// Takes the value by a universal reference and, due to perfect forwarding, both copy and
    /// move initialization is possible.
    explicit Box(T&& value) : m_ptr(std::make_unique<T>(std::forward<T>(value))) {}

    /// Constructor that adopts an existing `std::unique_ptr`.
    ///
    /// Takes the `std::unique_ptr` by value, so the latter must be either passed directly as an
    /// rvalue or `std::move()`d into the argument. However, please note that if your code snippet
    /// looks like this:
    ///
    /// ```cpp
    /// Box(std::make_unique<T>(args...))
    /// ```
    ///
    /// Then you should take a look at the `construct_in_place` method:
    ///
    /// ```cpp
    /// Box<T>::construct_in_place(args...)
    /// ```
    explicit Box(std::unique_ptr<T> ptr) : m_ptr(std::move(ptr)) {}

    /// Copy constructor. Creates another `std::unique_ptr` with a copy of the currently wrapped
    /// value.
    ///
    /// @param `other` the `Box` from which to copy.
    ///
    /// @throws BoxIsEmpty if `other` is empty.
    template <typename Dummy = T>
    Box(const Box& other) : m_ptr(std::make_unique<T>(*other)) {}

    /// Construct a value on the heap in place.
    ///
    /// Forwards the arguments to `std::make_unique` and wraps the resulting `std::unique_ptr`.
    template <typename... Args>
    static Box<T> construct_in_place(Args&&... args) {
        return Box(std::make_unique<T>(std::forward<Args>(args)...));
    }

    /// Derefencing operator: obtain a const reference to the stored value.
    ///
    /// @throws BoxIsEmpty if this `Box` does not contain a value at the moment.
    const T& operator*() const {
        if (m_ptr == nullptr) {
            throw BoxIsEmpty{};
        }
        return *m_ptr;
    }

    /// Derefencing operator: obtain a reference to the stored value.
    ///
    /// @throws BoxIsEmpty if this `Box` does not contain a value at the moment.
    T& operator*() {
        if (m_ptr == nullptr) {
            throw BoxIsEmpty{};
        }
        return *m_ptr;
    }

private:
    std::unique_ptr<T> m_ptr;
};

/// Type deduction guideline for `Box` (value initialization).
template <typename T>
Box(T&& value) -> Box<T>;

/// Type deduction guideline for `Box` (`std::unique_ptr` adoption).
template <typename T>
Box(std::unique_ptr<T> ptr) -> Box<T>;

template <typename T, typename U>
bool operator==(const Box<T>& lhs, const Box<U>& rhs) {
    return *lhs == *rhs;
}

template <typename T, typename U>
bool operator!=(const Box<T>& lhs, const Box<U>& rhs) {
    return !(lhs == rhs);
}

}  // namespace wr22::regex_parser::utils
