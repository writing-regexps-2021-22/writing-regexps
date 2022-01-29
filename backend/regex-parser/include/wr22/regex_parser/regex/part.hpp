#pragma once

// wr22
#include <wr22/regex_parser/regex/capture.hpp>
#include <wr22/regex_parser/utils/adt.hpp>
#include <wr22/regex_parser/utils/box.hpp>

// stl
#include <iosfwd>
#include <memory>
#include <vector>

namespace wr22::regex_parser::regex {

// Forward declaration of `Part`.
class Part;

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
    };

    /// An regex part that matches a single character literally.
    ///
    /// Corresponds to a plain character in a regular expression. E.g. the regex `"foo"` contains
    /// three character literals: `f`, `o` and `o`.
    struct Literal {
        explicit Literal(char32_t character);

        char32_t character;
        bool operator==(const Literal& rhs) const = default;
    };

    /// A regex part with the list of alternatives to be matched.
    ///
    /// Alternatives in regular expressions are subexpressions by `|`. For the whole expression
    /// part's match to succeed, at least one of the subexpressions must match the input
    /// successfully.
    ///
    /// As an example, `a|(b)|cde` would be represented as an `Alternatives` part with 3
    /// alternatives. The alternatives themselves are represented recursively as `Part`s.
    struct Alternatives {
        /// Constructor.
        ///
        /// Takes the vector of alternatives by value. Since `Part` cannot be copied, usage of
        /// copy-initialization or braced initializer list is not possible. Instead, either
        /// construct the vector of alternatives directly (as an rvalue), or `std::move()` it into
        /// the constructor argument.
        explicit Alternatives(std::vector<Part> alternatives);

        /// The list of the alternatives.
        std::vector<Part> alternatives;
        bool operator==(const Alternatives& rhs) const = default;
    };

    /// A regex part with the list of items to be matched one after another.
    ///
    /// Sequences in regular expressions are just subexpressions going directly one after another.
    /// As an example, `a[b-e].` is a sequence of 3 subexpressions: `a`, `[b-e]` and `.`.
    /// As an another example, `ab` is a sequence of 2 subexpressions: `a` and `b`.
    struct Sequence {
        /// Constructor.
        ///
        /// Takes the vector of items by value. Since `Part` cannot be copied, usage of
        /// copy-initialization or braced initializer list is not possible. Instead, either
        /// construct the vector of alternatives directly (as an rvalue), or `std::move()` it into
        /// the constructor argument.
        explicit Sequence(std::vector<Part> items);

        /// The list of the subexpressions.
        std::vector<Part> items;
        bool operator==(const Sequence& rhs) const = default;
    };

    /// A regex part that represents a group in parentheses.
    ///
    /// A group in regular expressions is virtually everything that is enclosed with parentheses:
    /// `(some group)`, `(?:blablabla)` and `(?P<group_name>group contents)` are all groups.
    ///
    /// A group has two main attributes: (1) how it is captured during matching and (2) the contents
    /// of the group. The contents is simply another `Part`. The capture behavior is expressed by a
    /// separate type `Capture`. See its docs for additional info, and take a look at
    /// <https://www.regular-expressions.info/brackets.html> for an introduction to or a recap of
    /// regex groups and capturing.
    struct Group {
        /// Convenience constructor.
        ///
        /// Takes the inner regex part by value. Since `Part` cannot be copied, `std::move()`ing the
        /// part into the constructor argument may be required.
        explicit Group(Capture capture, Part inner);

        /// Capture behavior.
        Capture capture;
        /// The smart pointer to the group contents.
        utils::Box<Part> inner;

        bool operator==(const Group& rhs) const = default;
    };

    using Adt = utils::Adt<Empty, Literal, Alternatives, Sequence, Group>;
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
class Part : public part::Adt {
public:
    using part::Adt::Adt;
};

/// Convert a `Part` to a textual representation and write it to an `std::ostream`.
std::ostream& operator<<(std::ostream& out, const Part& part);

}  // namespace wr22::regex_parser::regex
