// wr22
#include <wr22/regex_parser/regex/part.hpp>
#include <wr22/regex_parser/span/span.hpp>
#include <wr22/unicode/conversion.hpp>

// STL
#include <iterator>
#include <ostream>

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

part::CharacterClass::CharacterClass(CharacterClassData data) : data(std::move(data)) {}

std::ostream& operator<<(std::ostream& out, const SpannedPart& spanned_part) {
    auto span = spanned_part.span();
    spanned_part.part().visit(
        [&out, span](const part::Empty&) { fmt::print(out, "Empty [{}]", span); },
        [&out, span](const part::Literal& part) {
            out << '\'';
            {
                auto it = std::ostream_iterator<char>(out);
                wr22::unicode::to_utf8_write(it, part.character);
            }
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
            fmt::print(
                out,
                "Group [{}] {{ capture: {}, inner: {} }}",
                span,
                part.capture,
                *part.inner);
        },
        [&out, span](const part::Optional& part) {
            fmt::print(out, "Optional [{}] {{ {} }}", span, *part.inner);
        },
        [&out, span](const part::Plus& part) {
            fmt::print(out, "Plus [{}] {{ {} }}", span, *part.inner);
        },
        [&out, span](const part::Star& part) {
            fmt::print(out, "Star [{}] {{ {} }}", span, *part.inner);
        },
        [&out, span]([[maybe_unused]] const part::Wildcard& part) {
            fmt::print(out, "Wildcard [{}]", span);
        },
        [&out, span](const part::CharacterClass& part) {
            fmt::print(
                out,
                "CharacterClass [{}]{} {{ ",
                span,
                part.data.inverted ? " (inverted)" : "");
            bool first = true;
            for (const auto& range : part.data.ranges) {
                if (!first) {
                    out << ", ";
                }
                first = false;
                out << '\'' << wr22::unicode::to_utf8(range.first()) << '\'';
                if (!range.is_single_character()) {
                    out << '-';
                    out << '\'' << wr22::unicode::to_utf8(range.last()) << '\'';
                }
            }
            out << " }";
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

namespace part {
    void to_json(nlohmann::json& j, [[maybe_unused]] const part::Empty& part) {
        j = nlohmann::json::object();
    }

    void to_json(nlohmann::json& j, const part::Literal& part) {
        j = nlohmann::json::object();
        j["char"] = wr22::unicode::to_utf8(part.character);
    }

    void to_json(nlohmann::json& j, const part::Alternatives& part) {
        j = nlohmann::json::object();
        j["alternatives"] = part.alternatives;
    }

    void to_json(nlohmann::json& j, const part::Sequence& part) {
        j = nlohmann::json::object();
        j["items"] = part.items;
    }

    void to_json(nlohmann::json& j, const part::Group& part) {
        j = nlohmann::json::object();
        j["inner"] = *part.inner;
        j["capture"] = part.capture;
    }

    void to_json(nlohmann::json& j, const part::Optional& part) {
        j = nlohmann::json::object();
        j["inner"] = *part.inner;
    }

    void to_json(nlohmann::json& j, const part::Plus& part) {
        j = nlohmann::json::object();
        j["inner"] = *part.inner;
    }

    void to_json(nlohmann::json& j, const part::Star& part) {
        j = nlohmann::json::object();
        j["inner"] = *part.inner;
    }

    void to_json(nlohmann::json& j, [[maybe_unused]] const part::Wildcard& part) {
        j = nlohmann::json::object();
    }

    void to_json(nlohmann::json& j, const part::CharacterClass& part) {
        j = nlohmann::json::object();
        j["inverted"] = part.data.inverted;
        j["ranges"] = part.data.ranges;
    }
}  // namespace part

void to_json(nlohmann::json& j, const Part& part) {
    part.visit([&j](const auto& variant) { to_json(j, variant); });
    j["type"] = part.visit([](const auto& variant) { return variant.code_name; });
}

void to_json(nlohmann::json& j, const SpannedPart& part) {
    to_json(j, part.part());
    to_json(j["span"], part.span());
}

}  // namespace wr22::regex_parser::regex
