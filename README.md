# beman.scope: Generic Scope Guard

<!--
SPDX-License-Identifier: CC0-1.0
-->

[![Library Status](https://raw.githubusercontent.com/bemanproject/beman/refs/heads/main/images/badges/beman_badge-beman_library_under_development.svg)](https://github.com/bemanproject/beman/blob/main/docs/beman_library_maturity_model.md#the-beman-library-maturity-model) ![Continuous Integration Tests](https://github.com/bemanproject/scope/actions/workflows/ci_tests.yml/badge.svg) ![Lint Check (pre-commit)](https://github.com/bemanproject/scope/actions/workflows/pre-commit.yml/badge.svg) [![Coverage](https://coveralls.io/repos/github/bemanproject/scope/badge.svg?branch=main)](https://coveralls.io/github/bemanproject/scope?branch=main) ![Standard Target](https://github.com/bemanproject/beman/blob/main/images/badges/cpp29.svg) [![Compiler Explorer Example](https://img.shields.io/badge/Try%20it%20on%20Compiler%20Explorer-grey?logo=compilerexplorer&logoColor=67c52a)](https://godbolt.org/z/qMvrsPexd)

`beman.scope` is a C++ library that provides `scope_guard` facilities. The library conforms to [The Beman Standard](https://github.com/bemanproject/beman/blob/main/docs/beman_standard.md).

**Implements**: [D3610R0 Scope Guard](./papers/scope.org) targeted at C++29.

**Status**: [Under development and not yet ready for production use.](https://github.com/bemanproject/beman/blob/main/docs/beman_library_maturity_model.md#under-development-and-not-yet-ready-for-production-use)

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
- [Prior art, papers, related libraries, videos](https://github.com/bemanproject/scope/blob/main/docs/resources.md)

## Usage

- [![Compiler Explorer Example](https://img.shields.io/badge/Try%20it%20on%20Compiler%20Explorer-grey?logo=compilerexplorer&logoColor=67c52a)](https://godbolt.org/z/qMvrsPexd)

The following is an example of using `scope_fail` to trigger and action when the scope
is exited with an exception.  `scope_success` and `scope_exit` provide similar capability
but with different checked conditions on exiting the scope.

```c++
#include <beman/scope.hpp>


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

`unique_resource` is a customizable RAII type that can own nonpointer resources,
such as integer file descriptors. It closes the descriptor on scope exit,
including early returns and exception unwinding.

```c++
#include <beman/scope.hpp>
#include <cstdio>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {
    auto file = beman::scope::make_unique_resource_checked(
        ::open("example.txt", O_RDONLY), -1,
        [](int fd) noexcept { ::close(fd); });

    if (file.get() == -1) {
        std::perror("open");
        return 1; // No cleanup is attempted for the invalid descriptor.
    }

    struct stat info {};
    if (::fstat(file.get(), &info) == -1) {
        std::perror("fstat");
        return 1; // The descriptor is still closed on this early return.
    }

    // Use info; the descriptor is automatically closed when file leaves scope.
}
```

See the [runnable file-descriptor example](examples/unique_resource_fd.cpp)
and the [examples index](examples/examples.md).

## License

beman.scope is licensed under the Apache License v2.0 with LLVM Exceptions.

## Integrate beman.scope into your project

Beman.scope is a header-only library with its own scope guard and `unique_resource` implementations. The implementation requires C++20 or later.

Include `<beman/scope.hpp>` for all facilities, or include
`<beman/scope/scope_guard.hpp>` or `<beman/scope/unique_resource.hpp>` individually.

Note that modules support is currently tested only on clang++-19 and above and g++-15, and
is not supported if the C++ standard is below C++23.

As a header only library no building is required to use in a project -- simply make
the `include` directory available add add the following to your source.

```cpp
#include <beman/scope.hpp>

//modular version

import beman.scope;
```
With modules import needs to be after any includes to avoid compilation errors.

## Dependencies

### Build Environment

This project requires at least the following to build:

* A C++ compiler that conforms to the C++20 standard or greater
* CMake 3.30 or later
* (Test Only) Catch2

You can disable building tests by setting CMake option `BEMAN_SCOPE_BUILD_TESTS` to
`OFF` when configuring the project.

You can disable building examples by setting CMake option `BEMAN_SCOPE_BUILD_EXAMPLES` to
`OFF` when configuring the project.

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

### Supported Platforms

| Compiler | Version | C++ Standards | Standard Library |
|----------|---------|---------------|------------------|
| GCC      | 15-13   | C++26-C++20   | libstdc++        |
| Clang    | 20-19   | C++26-C++20   | libstdc++        |

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

Source is licensed with the Apache License v2.0 with LLVM Exceptions.

// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

Documentation and associated papers are licensed with the Creative Commons Attribution 4.0 International license.

// SPDX-License-Identifier: CC-BY-4.0

The intent is that the source and documentation are available for use by people how they wish.

The README itself is licensed with CC0 1.0 Universal. Copy the contents and incorporate in your own work as you see fit.

// SPDX-License-Identifier: CC0-1.0

# Contributing

Please do! Issues and pull requests are appreciated.
