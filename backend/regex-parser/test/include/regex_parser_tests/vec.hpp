#pragma once

// stl
#include <vector>

namespace regex_parser_tests {

namespace detail::vec {
    template <typename T>
    void append([[maybe_unused]] std::vector<T>& vec) {}

    template <typename T, typename Head, typename... Tail>
    void append(std::vector<T>& vec, Head&& head, Tail&&... tail) {
        vec.push_back(std::move(head));
        append(vec, std::move(tail)...);
    }
}  // namespace detail::vec

/// Make a vector with arguments as its elements.
///
/// `vec<T>(a, b, c, d, e)` is thus roughly equivalent to `std::vector<T>{a, b, c, d, e}` except that
/// the latter (initializer list) necessarily involves copying elements and does not work at all with
/// `T` that is not copy-constructable, while `vec<T>(...)` works with both copying and moving.
template <typename T, typename... Args>
std::vector<T> vec(T&& head, Args&&... tail) {
    std::vector<T> result;
    detail::vec::append(result, std::move(head), std::move(tail)...);
    return result;
}

}  // namespace regex_parser_tests
