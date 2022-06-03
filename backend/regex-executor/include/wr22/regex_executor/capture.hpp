#pragma once

// wr22
#include <wr22/regex_parser/span/span.hpp>

// stl
#include <iosfwd>
#include <string_view>
#include <unordered_map>

// nlohmann
#include <nlohmann/json_fwd.hpp>

namespace wr22::regex_executor {

struct Capture {
    regex_parser::span::Span string_span;
    constexpr bool operator==(const Capture& other) const = default;
};
void to_json(nlohmann::json& j, Capture capture);
std::ostream& operator<<(std::ostream& out, Capture capture);

struct Captures {
    Capture whole;
    std::unordered_map<size_t, Capture> indexed;
    std::unordered_map<std::string_view, Capture> named;
    bool operator==(const Captures& other) const = default;
};
void to_json(nlohmann::json& j, const Captures& captures);
std::ostream& operator<<(std::ostream& out, const Captures& captures);

}  // namespace wr22::regex_executor
