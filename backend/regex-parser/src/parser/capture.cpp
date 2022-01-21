// wr22
#include <wr22/regex_parser/regex/capture.hpp>

// STL
#include <iterator>
#include <ostream>

// boost
#include <boost/locale/utf.hpp>

namespace wr22::regex_parser::regex {

std::ostream& operator<<(std::ostream& out, const Capture& capture) {
    capture.visit(
        [&out](const capture::None&) { out << "None"; },
        [&out](const capture::Index&) { out << "Index"; },
        [&out](const capture::Name& capture) {
            out << "Name(" << capture.name << ")";
        });
    return out;
}

}  // namespace wr22::regex_parser::regex
