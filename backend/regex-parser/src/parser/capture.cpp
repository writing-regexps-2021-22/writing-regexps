// wr22
#include <wr22/regex_parser/regex/capture.hpp>
#include <wr22/regex_parser/regex/named_capture_flavor.hpp>

// STL
#include <iterator>
#include <ostream>

// fmt
#include <fmt/core.h>
#include <fmt/ostream.h>

namespace wr22::regex_parser::regex {

capture::Name::Name(std::string name, NamedCaptureFlavor flavor)
    : name(std::move(name)), flavor(flavor) {}

std::ostream& operator<<(std::ostream& out, const Capture& capture) {
    capture.visit(
        [&out](const capture::None&) { out << "None"; },
        [&out](const capture::Index&) { out << "Index"; },
        [&out](const capture::Name& capture) {
            fmt::format_to(
                std::ostreambuf_iterator<char>(out),
                "Name({}, {})",
                capture.name,
                capture.flavor);
        });
    return out;
}

void to_json(nlohmann::json& j, const Capture& capture) {
    capture.visit([&j](const auto& variant) { to_json(j, variant); });
    j["type"] = capture.visit([](const auto& variant) { return variant.code_name; });
}

namespace capture {
    void to_json(nlohmann::json& j, [[maybe_unused]] const None& capture) {
        j = nlohmann::json::object();
    }

    void to_json(nlohmann::json& j, [[maybe_unused]] const Index& capture) {
        j = nlohmann::json::object();
    }

    void to_json(nlohmann::json& j, const Name& capture) {
        j = nlohmann::json::object();
        j["name"] = capture.name;
        j["flavor"] = capture.flavor;
    }
}

}  // namespace wr22::regex_parser::regex
