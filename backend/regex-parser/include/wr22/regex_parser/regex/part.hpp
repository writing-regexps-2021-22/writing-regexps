#pragma once

// wr22
#include <wr22/regex_parser/utils/adt.hpp>

// stl
#include <variant>
#include <vector>

namespace wr22::regex_parser::regex {

class Part;

namespace part {
    struct Empty {
        bool operator==(const Empty& rhs) const = default;
    };

    struct Literal {
        char32_t character;
        bool operator==(const Literal& rhs) const = default;
    };

    struct Alternatives {
        std::vector<Part> alternatives;
        bool operator==(const Alternatives& rhs) const = default;
    };

    struct Sequence {
        std::vector<Part> items;
        bool operator==(const Sequence& rhs) const = default;
    };

    using Adt = utils::Adt<Empty, Literal, Alternatives, Sequence>;
}  // namespace part

class Part : public part::Adt {
public:
    using part::Adt::Adt;
    using part::Adt::operator==;
};

}  // namespace wr22::regex_parser::regex
