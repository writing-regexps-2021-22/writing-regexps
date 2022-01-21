#pragma once

// stl
#include <vector>

namespace regex_parser_tests {

namespace detail::vec {
    template <typename T>
    void append(std::vector<T>& vec) {}

    template <typename T, typename Head, typename... Tail>
    void append(std::vector<T>& vec, Head&& head, Tail&&... tail) {
        vec.push_back(std::move(head));
        append(vec, std::move(tail)...);
    }
}  // namespace detail::vec

template <typename T, typename... Args>
std::vector<T> vec(T&& head, Args&&... tail) {
    std::vector<T> result;
    detail::vec::append(result, std::move(head), std::move(tail)...);
    return result;
}

}  // namespace regex_parser_tests
