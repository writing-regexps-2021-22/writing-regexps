#pragma once

// wr22
#include <nlohmann/json_fwd.hpp>
#include <wr22/regex_parser/regex/capture.hpp>
#include <wr22/regex_parser/regex/character_class_data.hpp>
#include <wr22/regex_parser/span/span.hpp>
#include <wr22/regex_parser/utils/adt.hpp>
#include <wr22/regex_parser/utils/box.hpp>

// stl
#include <iosfwd>
#include <memory>
#include <vector>

// nlohmann
#include <nlohmann/json.hpp>

namespace wr22::regex_parser::regex {

// Forward declarations.
class Part;
class SpannedPart;

/// The namespace with the variants of `Part`.
///
/// See the docs for the `Part` type for additional information.
namespace part {
    /// An empty regex part.
    ///
    /// Corresponds to an empty regular expression (`""`) or the contents of an empty parenthesized
    /// group (`"()"`).
    struct Empty {
        explicit Empty() = default;
        bool operator==(const Empty& rhs) const = default;
        static constexpr const char* code_name = "empty";
    };
    void to_json(nlohmann::json& j, const Empty& part);

    /// An regex part that matches a single character literally.
    ///
    /// Corresponds to a plain character in a regular expression. E.g. the regex `"foo"` contains
    /// three character literals: `f`, `o` and `o`.
    struct Literal {
        explicit Literal(char32_t character);
        bool operator==(const Literal& rhs) const = default;
        static constexpr const char* code_name = "literal";

        char32_t character;
    };
    void to_json(nlohmann::json& j, const Literal& part);

    /// A regex part with the list of alternatives to be matched.
    ///
    /// Alternatives in regular expressions are subexpressions by `|`. For the whole expression
    /// part's match to succeed, at least one of the subexpressions must match the input
    /// successfully.
    ///
    /// As an example, `a|(b)|cde` would be represented as an `Alternatives` part with 3
    /// alternatives. The alternatives themselves are represented recursively as `SpannedPart`s.
    struct Alternatives {
        explicit Alternatives(std::vector<SpannedPart> alternatives);
        bool operator==(const Alternatives& rhs) const = default;
        static constexpr const char* code_name = "alternatives";

        /// The list of the alternatives.
        std::vector<SpannedPart> alternatives;
    };
    void to_json(nlohmann::json& j, const Alternatives& part);

    /// A regex part with the list of items to be matched one after another.
    ///
    /// Sequences in regular expressions are just subexpressions going directly one after another.
    /// As an example, `a[b-e].` is a sequence of 3 subexpressions: `a`, `[b-e]` and `.`.
    /// As an another example, `ab` is a sequence of 2 subexpressions: `a` and `b`.
    struct Sequence {
        explicit Sequence(std::vector<SpannedPart> items);
        bool operator==(const Sequence& rhs) const = default;
        static constexpr const char* code_name = "sequence";

        /// The list of the subexpressions.
        std::vector<SpannedPart> items;
    };
    void to_json(nlohmann::json& j, const Sequence& part);

    /// A regex part that represents a group in parentheses.
    ///
    /// A group in regular expressions is virtually everything that is enclosed with parentheses:
    /// `(some group)`, `(?:blablabla)` and `(?P<group_name>group contents)` are all groups.
    ///
    /// A group has two main attributes: (1) how it is captured during matching and (2) the contents
    /// of the group. The contents is simply another `SpannedPart`. The capture behavior is
    /// expressed by a separate type `Capture`. See its docs for additional info, and take a look at
    /// <https://www.regular-expressions.info/brackets.html> for an introduction to or a recap of
    /// regex groups and capturing.
    struct Group {
        /// Convenience constructor.
        explicit Group(Capture capture, SpannedPart inner);
        bool operator==(const Group& rhs) const = default;
        static constexpr const char* code_name = "group";

        /// Capture behavior.
        Capture capture;
        /// The smart pointer to the group contents.
        utils::Box<SpannedPart> inner;
    };
    void to_json(nlohmann::json& j, const Group& part);

    /// A regex part specifying an optional quantifier (`(expression)?`).
    struct Optional {
        /// Convenience constructor.
        explicit Optional(SpannedPart inner);
        bool operator==(const Optional& rhs) const = default;
        static constexpr const char* code_name = "optional";

        /// The smart pointer to the subexpression under the quantifier.
        utils::Box<SpannedPart> inner;
    };
    void to_json(nlohmann::json& j, const Optional& part);

    /// A regex part specifying an "at least one" quantifier (`(expression)+`).
    struct Plus {
        /// Convenience constructor.
        explicit Plus(SpannedPart inner);
        bool operator==(const Plus& rhs) const = default;
        static constexpr const char* code_name = "plus";

        /// The smart pointer to the subexpression under the quantifier.
        utils::Box<SpannedPart> inner;
    };
    void to_json(nlohmann::json& j, const Plus& part);

    /// A regex part specifying an "at least zero" quantifier (`(expression)*`).
    struct Star {
        /// Convenience constructor.
        explicit Star(SpannedPart inner);
        bool operator==(const Star& rhs) const = default;
        static constexpr const char* code_name = "star";

        /// The smart pointer to the subexpression under the quantifier.
        utils::Box<SpannedPart> inner;
    };
    void to_json(nlohmann::json& j, const Star& part);

    /// A regex part specifying any single character (`.`).
    struct Wildcard {
        explicit Wildcard() = default;
        bool operator==(const Wildcard& rhs) const = default;
        static constexpr const char* code_name = "wildcard";
    };
    void to_json(nlohmann::json& j, const Wildcard& part);

    /// A regex part specifying a character class (e.g. `[a-z_]`).
    struct CharacterClass {
        explicit CharacterClass(CharacterClassData data);
        bool operator==(const CharacterClass& rhs) const = default;
        static constexpr const char* code_name = "character_class";

        /// The list of character ranges.
        CharacterClassData data;
    };
    void to_son(nlohmann::json& j, const CharacterClass& part);

    using Adt = utils::
        Adt<Empty, Literal, Alternatives, Sequence, Group, Optional, Plus, Star, Wildcard>;
}  // namespace part

/// A part of a regular expression and its AST node type.
///
/// The parsed regular expressions are represented as abstract syntax trees (ASTs).
/// These are tree-like data structures where each node represents a regular expression
/// part (or the whole regex), and, depending on their type, these nodes may have subexpressions.
/// Subexpressions are `Part`s themselves, which also have child expressions and so on.
/// For example, `part::Sequence` has a number of subexpressions, and each of them is of the type
/// `Part` and is an AST node.
///
/// Each regex part has its own simple function. For example, `part::Alternatives` tries to match
/// several alternative subexpressions against the input and succeeds if at least one of them does;
/// and `part::Sequence` matches several subexpressions one after another, requiring them all to
/// match respective parts of the input. By combining these simple nodes, it becomes possible to
/// represent complex regular expressions. For example, the regex `aaa|bb` can be represented as a
/// `part::Alternatives`, where each of the alternatives is a `parts::Sequence` of `part::Literal`s.
///
/// The `Part` itself is represented by
/// [`std::variant`](https://en.cppreference.com/w/cpp/utility/variant) via the helper class
/// `utils::Adt`. In a nutshell, it allows a regex part to "have" one of the several predefined
/// types (the so-called variants, which are defined in the `part` namespace), but still be
/// represented as a `Part`. For the list of operations that can be performed on this type, e.g. to
/// check if an instance of `Parts` has a specific variant and, if yes, access the value of this
/// variant, see the documentation for the `utils::Adt` class, which `Part` inherits from.
///
/// Note that this type contains no span information for the root AST node. For a spanned version,
/// see `SpannedPart`.
class Part : public part::Adt {
public:
    using part::Adt::Adt;
};
void to_json(nlohmann::json& j, const Part& part);

/// A version of `Part` including the span information (position in the input) of the root AST node
/// (child nodes always contain it because they are represented as `SpannedPart`s themselves).
class SpannedPart {
public:
    explicit SpannedPart(Part part, span::Span span);

    bool operator==(const SpannedPart& other) const = default;
    bool operator!=(const SpannedPart& other) const = default;

    /// Access the wrapped `Part` (const version).
    const Part& part() const;
    /// Access the wrapped `Part` (non-const version).
    Part& part();

    /// Get the associated span.
    span::Span span() const;

private:
    Part m_part;
    span::Span m_span;
};
void to_json(nlohmann::json& j, const SpannedPart& part);

/// Convert a `SpannedPart` to a textual representation and write it to an `std::ostream`.
std::ostream& operator<<(std::ostream& out, const SpannedPart& part);

}  // namespace wr22::regex_parser::regex
