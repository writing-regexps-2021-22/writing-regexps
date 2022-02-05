// wr22
#include "wr22/regex_parser/span/span.hpp"
#include <wr22/regex_parser/regex/part.hpp>

// STL
#include <iterator>
#include <ostream>

// boost
#include <boost/locale/utf.hpp>

// fmt
#include <fmt/core.h>
#include <fmt/ostream.h>

namespace wr22::regex_parser::regex {

part::Literal::Literal(char32_t character) : character(character) {}

part::Alternatives::Alternatives(std::vector<SpannedPart> alternatives)
    : alternatives(std::move(alternatives)) {}

part::Sequence::Sequence(std::vector<SpannedPart> items) : items(std::move(items)) {}

part::Group::Group(Capture capture, SpannedPart inner)
    : capture(std::move(capture)), inner(utils::Box(std::move(inner))) {}

part::Optional::Optional(SpannedPart inner) : inner(utils::Box(std::move(inner))) {}

part::Plus::Plus(SpannedPart inner) : inner(utils::Box(std::move(inner))) {}

part::Star::Star(SpannedPart inner) : inner(utils::Box(std::move(inner))) {}

std::ostream& operator<<(std::ostream& out, const SpannedPart& spanned_part) {
    auto span = spanned_part.span();
    spanned_part.part().visit(
        [&out, span](const part::Empty&) { fmt::print(out, "Empty [{}]", span); },
        [&out, span](const part::Literal& part) {
            using utf_traits = boost::locale::utf::utf_traits<char>;
            out << '\'';
            auto code_point = static_cast<boost::locale::utf::code_point>(part.character);
            utf_traits::encode(code_point, std::ostream_iterator<char>(out));
            out << '\'';
            fmt::print(out, " [{}]", span);
        },
        [&out, span](const part::Alternatives& part) {
            fmt::print(out, "Alternatives [{}] {{ ", span);
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
        [&out, span](const part::Sequence& part) {
            fmt::print(out, "Sequence [{}] {{ ", span);
            bool first = true;
            for (const auto& item : part.items) {
                if (!first) {
                    out << ", ";
                }
                first = false;
                out << item;
            }
            out << " }";
        },
        [&out, span](const part::Group& part) {
            fmt::print(out, "Group [{}] {{ capture: {}, inner: {} }}", span, part.capture, *part.inner);
        },
        [&out, span](const part::Optional& part) {
            fmt::print(out, "Optional [{}] {{ {} }}", span, *part.inner);
        },
        [&out, span](const part::Plus& part) {
            fmt::print(out, "Plus [{}] {{ {} }}", span, *part.inner);
        },
        [&out, span](const part::Star& part) {
            fmt::print(out, "Star [{}] {{ {} }}", span, *part.inner);
        });
    return out;
}

SpannedPart::SpannedPart(Part part, span::Span span) : m_part(std::move(part)), m_span(span) {}

const Part& SpannedPart::part() const {
    return m_part;
}

Part& SpannedPart::part() {
    return m_part;
}

span::Span SpannedPart::span() const {
    return m_span;
}

}  // namespace wr22::regex_parser::regex
