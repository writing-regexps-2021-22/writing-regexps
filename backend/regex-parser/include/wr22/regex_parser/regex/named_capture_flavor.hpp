#pragma once

// stl
#include <iosfwd>

namespace wr22::regex_parser::regex {

enum class NamedCaptureFlavor
{
    Apostrophes,
    Angles,
    AnglesWithP,
};

std::ostream& operator<<(std::ostream& out, NamedCaptureFlavor flavor);

}  // namespace wr22::regex_parser::regex
