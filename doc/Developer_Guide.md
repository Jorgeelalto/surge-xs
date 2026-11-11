# Developer Guide

## Coding style

### Basics

- Follow the campground rule: leave changed code cleaner, clearer, and better commented.
- Follow the small-change rule: prefer the smallest change that solves the problem.
- Use spaces, not tabs. Code uses four-space indentation and `clang-format`; CI checks
  formatting in `linux-codequality`.
- `clang-format` does not format `CMakeLists.txt`. Use two-space indentation and no space
  between `if` and `(` in CMake files.

### Naming

- Prefer `static constexpr <type> UPPERCASE_VARIABLES` over `#define` constants.
- Use CamelCase for classes and lower camelCase for functions.
- Do not prefix members or statics with `s_`, `m_`, or similar.
- Prefer fully qualified namespaces and types such as `std::vector`.
- Do not use `using namespace` in headers or add `using namespace std`.
- Use namespaces, not classes, to group functions; see `UserInteractions.h`.
- Prefer descriptive names, such as `userMessageDeliveryPipe` over `umdp`.

### Comments

Document function declarations in a Doxygen-style format:

```cpp
class Example
{
    /**
     * Replace the printer.
     * @param printerName Name of the printer to replace.
     */
    void replacePrinter(std::string printerName);
}
```

Use multiline comments for longer inline explanations. Use single-line comments
sparingly. Comments should explain code for the next person who maintains it.

### Formatting with clang-format

Surge includes a `.clang-format` file. Install `clang-format` and `git-clang-format`;
on macOS, use `brew install clang-format`. Installation on Linux and Windows is left as
an exercise to the reader; update this guide if you complete it.

Format changes either before committing, after staging, with `git clang-format`, or after
making commits with `git clang-format main` to format all changes from `main`. Commit and
squash that formatting commit.

### Miscellaneous

- For platform-specific classes with parallel implementations, use `src/mac`,
  `src/windows`, and `src/linux`, with stubs on all three platforms. See
  `UserInteractions.h`. Use `#if MAC` when only a small code section differs.
- The only numbers that usually make sense are 0, 1, n, and infinity. Explain other
  numeric constants.
- Prefer `std::ostringstream` over `sprintf`.
- Use `#pragma once`, as in most existing code, rather than include guards.
