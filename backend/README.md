# Backend

The part that performs all the analytical work on the regular expressions: parsing, execution,
hints, etc. and exposes these operations via an HTTP interface.

## Building
A C++20-capable compiler and [CMake][tool.cmake] 3.12 or newer are required.  Also, the following
dependencies need to be be available:

- [Boost][lib.boost] (`Locale` module)
- [Catch2][lib.catch2] (optional; required only if unit tests are built)
- [crow][lib.crow]
- [fmt][lib.fmt]
- [spdlog][lib.spdlog]

It is expected that this project is built in a Unix-like environment. The ability to build under
Windows without using MSYS2, Cygwin or WSL2 is not guaranteed.

Building using the [Ninja][tool.ninja] backend is recommended, although is should be also possible
to use other backends (e.g. Unix Makefiles). Example commands to build this library are as follows:

```sh
mkdir build
cd build
cmake .. -G Ninja   # See notes below
ninja
```

To build the project once again, only the `ninja` command is sufficient (provided that you are
in the build directory at the moment).

Additional optional CMake flags may be useful in some cases:

- `-DCMAKE_BUILD_TYPE=[Debug/Release]`: Control the level of optimization and debug information
  emitted.
- `-DCMAKE_EXPORT_COMPILE_COMMANDS=1`: Generate a compilation database (`compile_commands.json`)
  that might be used by external tools (e.g. [YouCompleteMe][tool.ycm] or other code completers).

Additional optional CMake flags used by individual modules:

### regex-parser
- `-DREGEX_PARSER_BUILD_TESTS=1`: Enable building an executable with unit tests. Run it to
  check that the tests catch no bugs in the library. Requires `Catch2` to be available in
  the system.

[lib.boost]: https://www.boost.org
[lib.catch2]: https://github.com/catchorg/Catch2
[lib.crow]: https://crowcpp.org
[lib.fmt]: https://fmt.dev
[lib.spdlog]: https://github.com/gabime/spdlog
[tool.cmake]: https://cmake.org
