// wr22
#include <wr22/regex_parser/regex/capture.hpp>
#include <wr22/regex_parser/regex/named_capture_flavor.hpp>

// STL
#include <iterator>
#include <ostream>

// boost
#include <boost/locale/utf.hpp>

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

}  // namespace wr22::regex_parser::regex
