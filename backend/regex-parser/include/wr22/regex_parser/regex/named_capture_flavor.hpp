#pragma once

// stl
#include <iosfwd>

// nlohmann
#include <nlohmann/json.hpp>

namespace wr22::regex_parser::regex {

/// The flavor (dialect) of a named group capture.
///
/// The most common variants are included. This list is subject to extension if deemed necessary.
/// The source used as a reference is https://www.regular-expressions.info/named.html.
enum class NamedCaptureFlavor
{
    /// The flavor `(?'name'contents)`. Mostly used in C# and other .NET-oriented languages,
    /// although can also be found in certain versions Perl, Boost and elsewhere.
    Apostrophes,
    /// The flavor `(?<name>contents)`. Mostly used in C# and other .NET-oriented languages,
    /// although can also be found in certain versions Perl, Boost and elsewhere.
    Angles,
    /// The flavor `(?P<name>contents)`. Found in Python, PCRE and elsewhere.
    AnglesWithP,
};

std::ostream& operator<<(std::ostream& out, NamedCaptureFlavor flavor);
void to_json(nlohmann::json& j, NamedCaptureFlavor flavor);

}  // namespace wr22::regex_parser::regex
