# Developer Guide

## Coding style

### Basics

* Obey the 'campground rule' that the code you change should be better. Cleaner, better names, more comments, etc.
* At the same time, obey the 'small change' rule, which is: most of the time the smallest
change is the best change. So, editing a function with a variable name you don't like is OK
if that variable name is in 30 other places.
* Use spaces, not tabs
* The code is formatted with 4 spaces tab width, and we use `clang-format` to automatically style things as you type.
Any and all pull requests are verified against `clang-format` and if a PR doesn't adhere to the formatting rules we've set,
CI will fail (`linux-codequality` job). Make sure the diffs you submit for merging are compliant to our `clang-format`!
* `clang-format` is not able to format `CMakeLists` but some IDEs have their own formatting tools for it.
The chosen formatting style here is: 2 space indents/tabs and no space between `if` and following`()`.
If your IDE is set to auto-format cmake files make sure to configure it accordingly.

### Naming

* used to `#define` constants as `UPPERCASE_VARIABLES`, but prefer `static constexpr <type> UPPERCASE_VARIABLES`. 
* `class HaveCamelCaseNames`
* `void functionsAreCamelCaseWithLowerFirst`
* We are not using `s_` or `m_` or equivalent notations for members or statics
* Full namespaces are generally preferred over `using`. We are trying to use
`std::vector` over `vector` in the code.
* Don't do `using namespace` in header files. Don't `using namespace std` in new code (but it is in some existing code).
* Use namespaces, not classes to group functions. Check out how we implemented `UserInteractions.h`
* Long and descriptive names are good. `userMessageDeliveryPipe` is better than `umdp`.

### Comments

Comments for declarations of member functions or free functions aspires to javadoc/doxygen style.

```
class Example
{
    /**
     * replace the printer
     * @param printerName which printer to replace
     *
     * Replace the printer
     */
    void replacePrinter(std::string printerName);
}
```

Comments for bigger chunks of code inline use a multiline comment

```
   ...
   x = 3 + y;

   /*
   ** Now we have to implement the nasty search
   ** across the entire set of the maps of sets
   ** and there is no natural index, so use this
   ** full loop
   */
   for (auto q ...
```

Use single line comments sparingly, but where appropriate, feel free.

Generally: Comment your code. Someone coming after you will thank you. And that someone may be you!

### Format your code properly with clang-format

Surge ships with a `.clang-format` file which makes choices about how to format code. You can
use this to make sure your diffs are formatted properly. Here's how:

* install `clang-format` and `git-clang-format`. On macOS this can be done with
  `brew install clang-format`. On Linux and Windows it is left as an exercise to the reader.
  If you complete that exercise, please update this doc!
* Develop and so on as you normally would following the prescription in our [git howto](How%20to%20Git.md)
* Format the code at one of two development points:
  * Before you commit, but after you add, do a `git clang-format` and it will correct your code
  * Add a series of commits on your branch (because you are on branch of course to develop)
    then at the very end, `git clang-format main` will reformat all your changes from main.
    Commit and squash that commit.


### Miscellaneous

Occasionally, code needs an `#if MAC` but if you have entire classes with parallel implementations,
just put the implementations in the `src/mac` `src/windows` and `src/linux` directories and
let CMake pick the right one. This does mean that you need stubs on all three platforms to link.
Look at the `UserInteractions.h` example.

The only numbers which make sense in code are 0, 1, n, and infinity. If you are using a number other
than that, perhaps toss in a comment.

Prefer `std::ostringstream` and so on to `sprintf` and so on.

`#pragma once`, while not [really standard](https://en.wikipedia.org/wiki/Pragma_once), is used in
most of the code, so we are continuing to use it rather than `#ifdef` guards.
