// wr22
#include <wr22/regex_parser/regex/part.hpp>

// STL
#include <iterator>
#include <ostream>

// boost
#include <boost/locale/utf.hpp>

namespace wr22::regex_parser::regex {

std::ostream& operator<<(std::ostream& out, const Part& part) {
    part.visit(
        [&out](const part::Empty&) { out << "Empty"; },
        [&out](const part::Literal& part) {
            using utf_traits = boost::locale::utf::utf_traits<char>;
            out << '\'';
            auto code_point = static_cast<boost::locale::utf::code_point>(part.character);
            utf_traits::encode(code_point, std::ostream_iterator<char>(out));
            out << '\'';
        },
        [&out](const part::Alternatives& part) {
            out << "Alternatives { ";
            bool first = true;
            for (const auto& alt : part.alternatives) {
                if (!first) {
                    out << ", ";
                }
                first = false;
                out << alt;
            }
            out << " }";
        },
        [&out](const part::Sequence& part) {
            using utf_traits = boost::locale::utf::utf_traits<char>;
            out << "Sequence { ";
            bool first = true;
            for (const auto& item : part.items) {
                if (!first) {
                    out << ", ";
                }
                first = false;
                out << item;
            }
            out << " }";
        });
    return out;
}

}  // namespace wr22::regex_parser::regex
