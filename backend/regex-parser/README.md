# regex-parser

This is the library for parsing regular expressions. It takes a regular expression as a UTF-8
string and parses it into its syntax tree, throwing exceptions if the regular expression cannot
be parsed (has invalid syntax). The syntax tree consists of nodes, each representing a particular
element in the regex. Each node contains full information about the component of the regex it
is representing (e.g.  the characters in a character class `[0-9a-f]` or the type of the group
capture `(abc)` vs `(?:abc)` vs `(?<name>abc)`) and the location of this element in the regex string.

## Usage instructions

The function that does parsing is [`wr22::regex_parser::parser::parse_regex`][fn.parse_regex].
A brief example of its usage is as follows:

```cpp
// Assume `parse_regex` and `UnicodeStringView` are already in the scope.
auto regex = std::string("a(b|c)+");
auto syntax_tree = parse_regex(UnicodeStringView(regex));
std::cout << syntax_tree << std::endl;
```

`parse_regex` accepts a [`wr22::regex_parser::utils::UnicodeStringView`][t.unicode_string_view]
as an argument and returns a [`wr22::regex_parser::regex::SpannedPart`][t.spanned_part].
`UnicodeStringView` is a small wrapper around [`std::string_view`][std::string_view] (also
compatible with [`std::string`][std::string] and ordinary string literals, see the example for
details). It assumes that a string is UTF-8 encoded and allows to iterate over Unicode characters
(technically, code points) in this string, decoding them from UTF-8 on the fly. The characters
are represented as `char32_t`, so that any Unicode code point can fit into this representation.

Currently, it is impossible to obtain a character by its index in a `UnicodeStringView`. Since this
is tricky to implement both simply and efficiently, the way the library handles Unicode might be
changed later (e.g. by making `parse_regex` accept a [`std::u32string_view`][std::u32string_view]
instead).  Of course, basic indexing/slicing operations on a `UnicodeStringView` will be
available before this library is considered ready to be used. Please note that getting a character
with a specified index in an `std::string_view` that refers to this string is not equivalent:
for string encoded in UTF-8 `std::string_view` considers non-ASCII characters as a sequence
of several bytes, and indexes only bytes. For example, `std::string_view("Привет")[3] ==
'\x80'`, while (hypothetical) `UnicodeStringView("Привет")[3] == U'в'` (`U` indicates that
'в' is a `char32_t`, not `char`).

The `SpannedPart` returned from `parse_regex()` represents the syntax tree of the parsed
regular expression.  `SpannedPart` consists of two items:

- A [`Span`][t.span], which indicates what range of characters in the regex is covered by
  this node. See the API reference for details.
- A [`Part`][t.part], which represents one of the supported types of nodes.

`Part` is an algebraic, or variant data type (see [`Adt`][t.adt] or [`std::variant`][std::variant]
for an explanation). A `Part` represents a node in syntax tree. There are different types of
syntax tree nodes, which correspond to the [variants][v.part] of `Part`:

- Empty node (`part::Empty`). Represents an empty regular expression or the content
  of an empty group, e.g. the inner part of `()`.
- Character literal (`part::Literal`). Represents a single Unicode character (code point)
  that is matched literally, e.g. `a`, or `Ǽ`.
- Sequence of nodes (`part::Sequence`). Represents a sequence of smaller elements of a regex that
  will be matched one after another. For example, `abc` is represented as a sequence of three
  `Literal`s.
- Alternatives list (`part::Alternatives`). Represents a list of smaller elements of a regex at least
  some of which should match. The alternatives are separated by `|` in a regex. E.g. `a|b|cd` contains
  three alternatives: a `Literal` `a`, a `Literal` `b` and a `Sequence` of `Literal`s `c` and `d`.
- Group (`part::Group`). Represents a subexpression in parentheses, e.g. `(abc)`. A group might
  be capturing or non-capturing, and, if capturing, might capture the matched substring by name or by index.
  For additional and more detailed information, see the API reference.
- Quantifiers (`part::Optional`, `part::Star`, `part::Plus`). These nodes represent a quantifier over
  a subexpression (e.g. `(foo)?`, `.*` or `[a-z]+`).

More variants will be eventually added.

Each of the listed types is a type a syntax tree node might have. Either of these types may be contained
in a `Part`, since these types are `Part`'s variants. To check what type of a syntax tree node is contained
in a given `Part` and to access the stored value of this type, the method [`Part::visit`][m.part.visit]
exists (see the API reference for a usage example).

For a more detailed reference on the functions and data types available in this library,
we ask the reader to take a look at the [API reference][api].

## Library status
Currently, the library is not ready to be seriously used as a building block. Some prototyping can
be done now, but the library's interface may currently change without a warning, and not all concepts
that might be necessary for the library usage are implemented yet (e.g. indexing a `UnicodeStringView`
is not currently possible). The library code resides on the `feature/regex-parser-cpp` branch
and will be merged into `master`/`main` when it becomes ready to be used (the library's interface might
still change from time to time).

Utilities such as `Adt` or `UnicodeStringView` (if not removed) might split off into a separate utility
library if it becomes necessary to use them from other code as well. This might change their namespaces
and the header files that need to be included.

The level of regex support is as follows.

**Supported features**:

- Literal characters
- Groups (capture by index or by name (3 flavors) or none at all)
- Alternative lists (`a|bb|ccc`)
- Quantifiers `?`, `+` and `*` (greedy only)

**Unsupported features**:

- Wildcards (`.`)
- Character classes (`[a-z]`)
- Start of line / end of line (`^`, `$`)
- Repetitions (e.g. `(abc){3}` or `x{5,10}`)
- Escape sequences (e.g. `\n` or `\d`)
- Special character escaping
- Extended character classes (`[[:digit:]]`)
- Lazy or possessive quantifiers (e.g. `*?` or `*+`).
- Lookaround
- Backreferences
- Extensions for recursion
- Other features not explicitly mentioned

**Compatibility notes**:

- Group names are currently represented as UTF-8 encoded `std::string`s.
  It is unclear if we should change it.
- Group names are not checked for being valid (e.g. `:0::-` is accepted as a group name).
  This should probably stay as it is, and an additional verification might be done in a
  post-processing step using language-specific settings.

## Building
A C++20-capable compiler and [CMake][tool.cmake] 3.12 or newer are required.  Also, the following
dependencies need to be be available:

- [Boost][lib.boost] (`Locale` module)
- [fmt][lib.fmt]
- [Catch2][lib.catch2] (optional; required only if unit tests are built)

It is expected that this library is built in a Unix-like environment. The ability to build under
Windows without using MSYS2, Cygwin or WSL2 is not guaranteed.

Building using the [Ninja][tool.ninja] backend is recommended, although is should be also possible
to use other backends (e.g. Unix Makefiles). Example commands to build this library are as follows:

```sh
mkdir build
cd build
cmake .. -G Ninja   # See notes below
ninja
```

To build the library once again, only the `ninja` command is sufficient (provided that you are
in the build directory at the moment).

Additional optional CMake flags may be useful in some cases:

- `-DCMAKE_BUILD_TYPE=[Debug/Release]`: Control the level of optimization and debug information
  emitted.
- `-DCMAKE_EXPORT_COMPILE_COMMANDS=1`: Generate a compilation database (`compile_commands.json`)
  that might be used by external tools (e.g. [YouCompleteMe][tool.ycm] or other code completers).
- `-DREGEX_PARSER_BUILD_TESTS=1`: Enable building an executable with unit tests. Run it to
  check that the tests catch no bugs in the library. Requires `Catch2` to be available in
  the system.

[api]: https://writing-regexps-2021-22.github.io/docs/regex-parser/index.html
[fn.parse_regex]: https://writing-regexps-2021-22.github.io/docs/regex-parser/namespacewr22_1_1regex__parser_1_1parser.html#a0dc595a19e81abed1c444fda1bbe6aee
[lib.boost]: https://www.boost.org/
[lib.catch2]: https://github.com/catchorg/Catch2
[lib.fmt]: https://fmt.dev
[m.part.visit]: https://writing-regexps-2021-22.github.io/docs/regex-parser/classwr22_1_1regex__parser_1_1utils_1_1Adt.html#a07d5c8e3b851046fa584fe4d8ec311ea
[std::string]: https://en.cppreference.com/w/cpp/string/basic_string
[std::string_view]: https://en.cppreference.com/w/cpp/string/basic_string_view
[std::u32string_view]: https://en.cppreference.com/w/cpp/string/basic_string_view
[std::variant]: https://en.cppreference.com/w/cpp/utility/variant
[t.adt]: https://writing-regexps-2021-22.github.io/docs/regex-parser/classwr22_1_1regex__parser_1_1utils_1_1Adt.html
[t.part]: https://writing-regexps-2021-22.github.io/docs/regex-parser/classwr22_1_1regex__parser_1_1regex_1_1Part.html
[t.span]: https://writing-regexps-2021-22.github.io/docs/regex-parser/classwr22_1_1regex__parser_1_1span_1_1Span.html
[t.spanned_part]: https://writing-regexps-2021-22.github.io/docs/regex-parser/classwr22_1_1regex__parser_1_1regex_1_1SpannedPart.html
[t.unicode_string_view]: https://writing-regexps-2021-22.github.io/docs/regex-parser/classwr22_1_1regex__parser_1_1utils_1_1UnicodeStringView.html
[tool.cmake]: https://cmake.org
[tool.ninja]: https://ninja-build.org
[tool.ycm]: https://github.com/ycm-core/YouCompleteMe
[v.part]: https://writing-regexps-2021-22.github.io/docs/regex-parser/namespacewr22_1_1regex__parser_1_1regex_1_1part.html
