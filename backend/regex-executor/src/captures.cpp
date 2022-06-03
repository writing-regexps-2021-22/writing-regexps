// wr22
#include <wr22/regex_executor/capture.hpp>

// nlhohmann
#include <nlohmann/json.hpp>

// stl
#include <ostream>
#include <unordered_map>

// fmt
#include <fmt/compile.h>
#include <fmt/core.h>
#include <fmt/ostream.h>

namespace wr22::regex_executor {

template <typename K, typename V>
struct MapPrinter {
    MapPrinter(const std::unordered_map<K, V>& map) : map(map) {}

    const std::unordered_map<K, V>& map;
};

template <typename K, typename V>
std::ostream& operator<<(std::ostream& out, MapPrinter<K, V> printer) {
    fmt::print(out, FMT_STRING("map ["));
    bool first = true;
    for (const auto& [key, value] : printer.map) {
        fmt::print(out, FMT_STRING("{}{} => {}"), first ? "" : ", ", key, value);
        first = false;
    }
    fmt::print(out, FMT_STRING("]"));
    return out;
}

void to_json(nlohmann::json& j, Capture capture) {
    j = nlohmann::json::object();
    j["string_span"] = capture.string_span;
}

std::ostream& operator<<(std::ostream& out, Capture capture) {
    fmt::print(out, FMT_STRING("Capture {{ {} }}"), capture.string_span);
    return out;
}

void to_json(nlohmann::json& j, const Captures& captures) {
    j = nlohmann::json::object();
    j["whole"] = captures.whole;
    j["by_name"] = captures.named;
    auto& by_index = j["by_index"];
    by_index = nlohmann::json::object();
    for (const auto& [index, capture] : captures.indexed) {
        by_index[std::to_string(index)] = capture;
    }
}

std::ostream& operator<<(std::ostream& out, const Captures& captures) {
    fmt::print(
        out,
        FMT_STRING("Captures {{ whole = {}, indexed = {}, named = {} }}"),
        captures.whole,
        MapPrinter(captures.indexed),
        MapPrinter(captures.named));
    return out;
}

}  // namespace wr22::regex_executor
