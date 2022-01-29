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

/// Group capture behavior.
///
/// A group can be captured by index (when one writes `(contents)`), by name (e.g.
/// `(?<name>contents)` in some dialects) or not captured at all (`(?:contents)`). Objects of this
/// type determine how exactly a certain group is going to be captured.
//
/// This is a variant type (see `Part` and  `utils::Adt` for a more detailed explanation of the
/// concept). The variants for this class (explicitly or implicitly convertible to this type) are
/// located in the `capture` namespace.
class Capture : public capture::Adt {
public:
    using capture::Adt::Adt;
};

std::ostream& operator<<(std::ostream& out, const Capture& capture);

}  // namespace wr22::regex_parser::regex
