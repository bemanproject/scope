<!--
SPDX-License-Identifier: CC0-1.0
-->

# beman.scope: Generic Scope Guard

![Library Status](https://raw.githubusercontent.com/bemanproject/beman/refs/heads/main/images/badges/beman_badge-beman_library_under_development.svg)
![Continuous Integration Tests](https://github.com/bemanproject/scope/actions/workflows/ci_tests.yml/badge.svg)
![Lint Check (pre-commit)](https://github.com/bemanproject/scope/actions/workflows/pre-commit.yml/badge.svg)

`beman.scope` is a C++ library provides `scope_guard` facilities. The library conforms to [The Beman Standard](https://github.com/bemanproject/beman/blob/main/docs/BEMAN_STANDARD.md).

**Implements**: [D3610R0 Scope Guard](./paper/scope.org) targeted at C++29.

**Status**: [Under development and not yet ready for production use.](https://github.com/bemanproject/beman/blob/main/docs/BEMAN_LIBRARY_MATURITY_MODEL.md#under-development-and-not-yet-ready-for-production-use)

# Overview

During the C++20 cycle [P0052 Generic Scope Guard and RAII Wrapper for the Standard Library](https://wg21.link/P0052)
added 4 types: `scope_exit`, `scope_fail`, `scope_success`
and `unique_resource` to [LTFSv3](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/n4908#scopeguard).

In the intervening time, two standard libraries have implemented support as well as Boost.
With the imperative for safety and security in C++ developers need every tool in the toolbox.
The authors believe it is time to move this facility into the standard.
The paper will re-examine the five year old design and any learning from deployment of the LTFSv3.

For discussions of this library and related work see:

- [Discourse for discussion of scope](https://discourse.bemanproject.org/t/scope-library/315)
- [Prior art, papers, related libraries, videos](https://github.com/bemanproject/scope/blob/main/resources.md)

## Usage

- [![Compiler Explorer Example](https://img.shields.io/badge/Try%20it%20on%20Compiler%20Explorer-grey?logo=compilerexplorer&logoColor=67c52a)](https://godbolt.org/z/qMvrsPexd)

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
and is thus currently available only on g++-13 and up, or clang 19 and up -- in C++20 mode.

Note that modules support is currently tested only on clang++-19 and above and g++-15.

As a header only library no building is required to use in a project -- simply make
the `include` directory available add add the following to your source.

```cpp
#include <beman/scope/scope.hpp>

//modular version

import beman.scope;
```
With g++-15 modules is import needs to be after any includes to avoid compilation errors.

## Building beman.scope

Building is only required to run tests and examples. All compilers build and
run `include` based tests. Compilers known to support modules are automatically
detected added to tests.

### Build Dependencies

The library itself has no build dependencies other than Catch2 for testing.
And for building cmake and ninja.  Makefiles are supported in non-modular builds.

Build-time dependencies:

- `cmake`
- `ninja`, `make`, or another CMake-supported build system
  - CMake defaults to "Unix Makefiles" on POSIX systems
- `catch2` for building tests

### How to build beman.scope tests and examples

from root of repo:

```shell
mkdir build; cd build;
cmake .. -DCMAKE_CXX_COMPILER=g++-15 -DCMAKE_CXX_STANDARD=26 -G=Ninja
ninja -j 5 -v; ctest
```

or using cmake presets
```shell
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
