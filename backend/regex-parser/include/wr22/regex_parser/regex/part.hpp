#pragma once

// wr22
#include <wr22/regex_parser/regex/capture.hpp>
#include <wr22/regex_parser/utils/adt.hpp>

// stl
#include <iosfwd>
#include <memory>
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

    struct Group {
        Capture capture;
        std::unique_ptr<Part> inner;
        bool operator==(const Group& rhs) const;
    };

    using Adt = utils::Adt<Empty, Literal, Alternatives, Sequence, Group>;
}  // namespace part

class Part : public part::Adt {
public:
    using part::Adt::Adt;
};

std::ostream& operator<<(std::ostream& out, const Part& part);

}  // namespace wr22::regex_parser::regex
