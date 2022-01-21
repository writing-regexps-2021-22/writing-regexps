// wr22
#include <wr22/regex_parser/regex/named_capture_flavor.hpp>

// STL
#include <ostream>

namespace wr22::regex_parser::regex {

std::ostream& operator<<(std::ostream& out, NamedCaptureFlavor flavor) {
    switch (flavor) {
    case NamedCaptureFlavor::Apostrophes:
        out << "Apostrophes";
        break;
    case NamedCaptureFlavor::Angles:
        out << "Angles";
        break;
    case NamedCaptureFlavor::AnglesWithP:
        out << "AnglesWithP";
        break;
    }
    return out;
}

}  // namespace wr22::regex_parser::regex
