#pragma once

// wr22
#include <wr22/regex_parser/regex/named_capture_flavor.hpp>
#include <wr22/regex_parser/utils/adt.hpp>

// stl
#include <iosfwd>
#include <string>

// nlohmann
#include <nlohmann/json.hpp>

namespace wr22::regex_parser::regex {

class Capture;

namespace capture {
    /// Denotes an non-capturing group.
    struct None {
        explicit None() = default;
        bool operator==(const None& rhs) const = default;
        static constexpr const char* code_name = "none";
    };
    void to_json(nlohmann::json& j, const None& capture);

    /// Denotes a group captured by index.
    struct Index {
        explicit Index() = default;
        bool operator==(const Index& rhs) const = default;
        static constexpr const char* code_name = "index";
    };
    void to_json(nlohmann::json& j, const Index& capture);

    /// Denotes a group captured by name.
    ///
    /// A specific name and the syntax variant for this name's specification (see
    /// `NamedCaptureFlavor`) are stored.
    struct Name {
        explicit Name(std::string name, NamedCaptureFlavor flavor);

        std::string name;
        NamedCaptureFlavor flavor;
        bool operator==(const Name& rhs) const = default;
        static constexpr const char* code_name = "name";
    };
    void to_json(nlohmann::json& j, const Name& capture);

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
void to_json(nlohmann::json& j, const Capture& capture);

}  // namespace wr22::regex_parser::regex
