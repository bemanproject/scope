<!--
SPDX-License-Identifier: CC0-1.0
-->

# beman.scope: Generic Scope Guard

![Library Status](https://github.com/bemanproject/beman/blob/c6997986557ec6dda98acbdf502082cdf7335526/images/badges/beman_badge-beman_library_under_development.svg)
![Continuous Integration Tests](https://github.com/bemanproject/scope/actions/workflows/ci_tests.yml/badge.svg)
![Lint Check (pre-commit)](https://github.com/bemanproject/scope/actions/workflows/pre-commit.yml/badge.svg)

# Overview

During the C++20 cycle [P0052 Generic Scope Guard and RAII Wrapper for the Standard Library](https://wg21.link/P0052)
added 4 types: `scope_exit`, `scope_fail`, `scope_success`
and `unique_resource` to [LTFSv3](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/n4908#scopeguard).
In the intervening time, two standard libraries have implemented support as well as Boost.
With the imperative for safety and security in C++ developers need every tool in the toolbox.
The authors believe it is time to move this facility into the standard.
The paper will re-examine the five year old design and any learning from deployment of the LTFSv3.

For discussions of this library see:

- [Discourse for discussion of scope](https://discourse.bemanproject.org/t/scope-library/315)

# Prior And Other Work

## Papers

- TS design and wording paper [p0052 - Generic Scope Guard and RAII Wrapper for the Standard Library](https://wg21.link/p0052)
- TS adoption paper [p1411 - Please reconsider <scope> for C++20](https://wg21.link/p1411)
- [N3677 A Proposal to Add additional RAII Wrappers to the Standard Library](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3677.html)
- [N4152 uncaught_exceptions - Sutter](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4152.pdf)

## Implementations

- [GSL final_action](https://github.com/microsoft/GSL/blob/main/include/gsl/util) - part of core guidelines
- [Boost.scope](https://www.boost.org/doc/libs/1_87_0/libs/scope/doc/html/index.html)
- [scope_guard based on Andrei Alexandrescu and Petru Marginean article](https://ricab.github.io/scope_guard)
- [Windows Implementation Libraries (WIL) - scope_exit](https://github.com/microsoft/wil/blob/182e6521140174e1d2ed1920f88d005fc4c546e2/include/wil/resource.h#L660)
- [GCC libstdc++-v3 experimental/scope implementation](https://gcc.gnu.org/git/?p=gcc.git;a=blob;f=libstdc%2B%2B-v3/include/experimental/scope;h=6e1d342e1b6486b0d1f32166c7eb91d29ed79f4d;hb=refs/heads/master)
- [LLVM - ADT/ScopeExit.h](https://github.com/llvm/llvm-project/blob/main/llvm/include/llvm/ADT/ScopeExit.h)
- [libcxx - scope_guard.h](https://github.com/llvm/llvm-project/blob/main/libcxx/include/__utility/scope_guard.h)
- [Folly - ScopeGuard.h](https://github.com/facebook/folly/blob/main/folly/ScopeGuard.h)
- [BDE (Bloomberg) - ScopeExit.h](https://github.com/bloomberg/bde/blob/main/groups/bdl/bdlb/bdlb_scopeexit.h)

## Videos

- [Peter Sommerlad - Woes of Scope Guards and Unique_Resource - 5+ years in the making](https://www.youtube.com/watch?v=O1sK__G5Nrg)
- [Andrei Alexandrescu - Declarative Control Flow](https://www.youtube.com/watch?v=WjTrfoiB0MQ)

# Examples

- [TS example of scope_exit](https://godbolt.org/z/T5KhTYjP7)

---

`beman.scope` is a C++ library conforming to [The Beman Standard](https://github.com/bemanproject/beman/blob/main/docs/BEMAN_STANDARD.md).

**Implements**: D3610R0 Scope Guard for C++29

**Status**: [Under development and not yet ready for production use.](https://github.com/bemanproject/beman/blob/main/docs/BEMAN_LIBRARY_MATURITY_MODEL.md#under-development-and-not-yet-ready-for-production-use)

## Usage

The following is an example of using `scope_fail` to trigger and action when the scope
is exited with an exception.  `scope_success` and `scope_exit` provide similar capability
but with different checked conditions on exiting the scope.

```c++
#include <beman/scope/scope.hpp>


    bool triggered = false;
    {
        scope_fail guard([&]() { triggered = true; });
        // no exception thrown
    }
    // triggered == false
    try {
        scope_fail guard([&]() { triggered = true; });

        throw std::runtime_error( "trigger failure" );

    } catch (...) { // expected }

    // triggered == true
```

`unique_resource` is a cutomizeable RAII type similar to `unique_ptr`.

```c++
#include <beman/scope/scope.hpp>

  {
    auto file = beman::scope::unique_resource(
        fopen("example.txt", "w"), // function to acquire the FILE*
        [](FILE* f) {              // function to cleanup on destruction
            if (f) {
                fclose(f); // Release (cleanup) the resource
            }
        }
    );

    // use file via f->
  }

  // Resource is automatically released when `file` goes out of scope
  std::cout << "File has been closed \n";
```

Full runnable examples can be found in `examples/`.

## Integrate beman.scope into your project

Beman.scope is a header-only library that currently relies on TS implementations
and is thus currently available only on GCC13 and up, or Clang 19 and up -- in C++20 mode.

As a header only library no building is required to use in a project -- simply make
the `include` directory available add add the following to your source.

```cpp
#include <beman/scope/scope.hpp>
```

## Building beman.scope

Building is only required to run tests and examples.

### Build Dependencies

The library itself has no build dependencies other than Catch2 for testing
and cmake.

Build-time dependencies:

- `cmake`
- `ninja`, `make`, or another CMake-supported build system
  - CMake defaults to "Unix Makefiles" on POSIX systems

### How to build beman.scope

```shell
cmake --workflow --preset gcc-debug
cmake --workflow --preset gcc-release
cmake --install build/gcc-release --prefix /opt/beman.scope
```

# License

Source is licensed with the Apache 2.0 license with LLVM exceptions

// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

Documentation and associated papers are licensed with the Creative Commons Attribution 4.0 International license.

// SPDX-License-Identifier: CC-BY-4.0

The intent is that the source and documentation are available for use by people how they wish.

The README itself is licensed with CC0 1.0 Universal. Copy the contents and incorporate in your own work as you see fit.

// SPDX-License-Identifier: CC0-1.0

# Contributing

Please do! Issues and pull requests are appreciated.
