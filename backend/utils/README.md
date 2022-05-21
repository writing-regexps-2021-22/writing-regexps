# utils
A library providing helper definitions and implementations other components of the project rely upon.
Currently, it provides the following features:

1. `Adt` — a wrapper around `std::variant` with improved support for matching. Serves as base for algebraic
   data types in the project.
2. `Box` — a safer wrapper around `std::unique_ptr` that never throws exception instead of
   returning/dereferencing a null pointer.
