#pragma once

// wr22
#include <wr22/regex_parser/utils/adt.hpp>

// stl
#include <iosfwd>
#include <string>

namespace wr22::regex_parser::regex {

class Capture;

namespace capture {
    struct None {
        bool operator==(const None& rhs) const = default;
    };

    struct Index {
        bool operator==(const Index& rhs) const = default;
    };

    struct Name {
        std::string name;
        // TODO: flavor.
        bool operator==(const Name& rhs) const = default;
    };

    using Adt = utils::Adt<None, Index, Name>;
}  // namespace capture

class Capture : public capture::Adt {
public:
    using capture::Adt::Adt;
};

std::ostream& operator<<(std::ostream& out, const Capture& capture);

}  // namespace wr22::regex_parser::regex
