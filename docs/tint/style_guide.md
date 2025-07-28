# Tint style guide

* Generally, follow the [Chromium style guide for C++](https://chromium.googlesource.com/chromium/src/+/HEAD/styleguide/c++/c++.md)
  which itself is built on the [Google C++ style guide](https://google.github.io/styleguide/cppguide.html).

* Overall try to use the same style and convention as code around your change.

* Code must be formatted. Use `clang-format` with the provided [.clang-format](../.clang-format)
  file.  The `tools/format` script runs the formatter.

* Code should not have linting errors.
    The `tools/lint` script runs the linter. So does `git cl upload`.

* Do not use C++ exceptions

* Do not use C++ RTTI.
   Instead, use `tint::Castable::As<T>()` from
   [src/castable.h](../src/castable.h)

* Generally, avoid `assert`.  Instead, issue a [diagnostic](../src/diagnostic.h)
  and fail gracefully, possibly by returning an error sentinel value.
  Code that should not be reachable should call `TINT_UNREACHABLE` macro
  and other internal error conditions should call the `TINT_ICE` macro.
  See [src/debug.h](../src/debug.h)

* Use `type` as part of a name only when the name refers to a type
  in WGSL or another shader language processed by Tint.  If the concept you are
  trying to name is about distinguishing between alternatives, use `kind` instead.

* Forward declarations:
  * Use forward declarations where possible, instead of using `#include`'s.
  * Place forward declarations in their own **un-nested** namespace declarations. \
    Example: \
    to forward-declare `struct X` in namespace `A` and `struct Y`
    in namespace `A::B`, you'd write:
    ```c++
    // Forward declarations
    namespace A {
      struct X;
    }  // namespace A
    namespace A::B {
      struct Y;
    }  // namespace A::B

    // rest of the header code is declared below ...
    ```

## Compiler support

Tint requires C++20.

Tint uses the Chromium build system and will stay synchronized with that system.
Compiler configurations beyond that baseline is on a best-effort basis.
We strive to support recent GCC and MSVC compilers.

## Test code

We might relax the above rules for test code, since test code
shouldn't ship to users.

However, test code should still be readable and maintainable.

For test code, the tradeoff between readability and maintainability
and other factors is weighted even more strongly toward readability
and maintainability.

## Infra/Tooling

Tint has infra scripts and tooling written in two primary languages
Python and Go.

The general advice when making changes to existing code is to use the
language that the existing implementation uses, since there is no
ongoing effort to migrate to a single common language, i.e. don't go
rewriting things just for the sake of rewriting things.

If you are looking at writing a new tool or script, where and how this
code is going to be used should guide your choice.

For example if it is a standalone tool that primarily lives in the
Dawn repo, especially for something like a CLI dev tool, etc, the
preference is to use Go, since there is a substantial existing body of
support code already.

If it is something that needs to directly interface with Chromium
tooling, e.g. interacts with GN or other support libraries/tools will
be accessing it, the preference would be to use Python.

That being said for many Chromium infrastructure tasks, i.e. running
in a build recipe, the integration there is via exec, even for Python
scripts. For integration there is no significant difference between
call into a Python script vs a Go one.

The primary driver in this case to using Python instead of Go would be
either something else written in Python that needs to import your code
as a library (and not just treat it like a standalone binary) or your
code needs to import Chromium specific Python code/libraries.

Often tooling for Tint does double duty, being a CLI for devs and also
how the bots do a similar task. Due to the above considerations,
i.e. existing support for CLI in Go and needing to fork regardless of
the language, the preference in these cases tends to be use Go if
possible.

Another way to view the decision is looking at the number of execs or
similar calls that need to occur. If you are designing something and
seeing that your code is going to be ping ponging between the two
languages via execs, it is probably worthwhile seeing if it is
feasible to implement everything as a library in one language.
