<!--
SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
-->

# beman.scope: Generic Scope Guard

![Library Status](https://github.com/bemanproject/beman/blob/c6997986557ec6dda98acbdf502082cdf7335526/images/badges/beman_badge-beman_library_under_development.svg)
![Continuous Integration Tests](https://github.com/bemanproject/scope/actions/workflows/ci_tests.yml/badge.svg)
![Lint Check (pre-commit)](https://github.com/bemanproject/scope/actions/workflows/pre-commit.yml/badge.svg)

# Overview

During the C++20 cycle [P0052 Generic Scope Guard and RAII Wrapper for the Standard Library](https://wg21.link/P0052) added 4 types: `scope_exit`, `scope_fail`, `scope_success` and `unique_resource` to [LTFSv3](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/n4908#scopeguard).  In the intervening time, two standard libraries have implemented support as well as Boost. With the imperative for safety and security in C++ developers need every tool in the toolbox. The authors believe it is time to move this facility into the standard.  The paper will re-examine the five year old design and any learning from deployment of the LTFSv3.

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

## Videos

- [Peter Sommerlad - Woes of Scope Guards and Unique_Resource - 5+ years in the making](https://www.youtube.com/watch?v=O1sK__G5Nrg)
- [Andrei Alexandrescu - Declarative Control Flow](https://www.youtube.com/watch?v=WjTrfoiB0MQ)

# Examples

- [TS example of scope_exit](https://godbolt.org/z/T5KhTYjP7)

---

`beman.scope` is a C++ library conforming to [The Beman Standard](https://github.com/bemanproject/beman/blob/main/docs/BEMAN_STANDARD.md).

**Implements**: TODO

**Status**: [Under development and not yet ready for production use.](https://github.com/bemanproject/beman/blob/main/docs/BEMAN_LIBRARY_MATURITY_MODEL.md#under-development-and-not-yet-ready-for-production-use)

## Usage

TODO

### Example Usage

TODO

Full runnable examples can be found in `examples/`.

## Building beman.scope

### Dependencies

This project has no C or C++ dependencies.

Build-time dependencies:

- `cmake`
- `ninja`, `make`, or another CMake-supported build system
  - CMake defaults to "Unix Makefiles" on POSIX systems

#### How to install dependencies

<!-- TODO Darius: rewrite section!-->

<details>
<summary>Dependencies install scope on Ubuntu 24.04  </summary>

<!-- TODO Darius: rewrite section!-->

```shell
# Install tools:
apt-get install -y cmake make ninja-build

# Toolchains:
apt-get install                           \
  g++-14 gcc-14 gcc-13 g++-14             \
  clang-18 clang++-18 clang-17 clang++-17
```

</details>

<details>
<summary>Dependencies install scope on MAC OS $VERSION </summary>

<!-- TODO Darius: rewrite section!-->
```shell
# TODO
```

</details>

<details>
<summary>Dependencies install scope on Windows $VERSION  </summary>
<!-- TODO Darius: rewrite section!-->

```shell
# TODO
```

</details>

### How to build beman.scope

Beman scope is header only.

```shell
cmake --workflow --preset gcc-debug
cmake --workflow --preset gcc-release
cmake --install build/gcc-release --prefix /opt/beman.scope
```

<details>
<summary> Build beman.scope (verbose logs) </summary>

```shell
# Configure beman.scope via gcc-debug workflow for development.
$ cmake --workflow --preset gcc-debug
Executing workflow step 1 of 3: configure preset "gcc-debug"

Preset CMake variables:

  CMAKE_BUILD_TYPE="Debug"
  CMAKE_CXX_COMPILER="g++"
  CMAKE_CXX_FLAGS="-fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=leak -fsanitize=undefined"
  CMAKE_CXX_STANDARD="20"

TODO

# Run examples.
$ TODO

```

</details>

<details>
<summary> Install beman.scope (verbose logs) </summary>

```shell
# Install build artifacts from `build` directory into `opt/beman.scope` path.
$ cmake --install build/gcc-release --prefix /opt/beman.scope
-- Install configuration: "RelWithDebInfo"
-- Up-to-date: /opt/beman.scope/lib/libbeman.exemplar.a
-- Up-to-date: /opt/beman.scope/include/beman/exemplar/identity.hpp


# Check tree.
$ tree /opt/beman.scope
/opt/beman.scope
├── include
│   └── beman
│       └── scope
│           └── scope.hpp


4 directories, 2 files
```

</details>

<details>
<summary> Disable tests build </summary>

To build this project with tests disabled (and their dependencies),
simply use `BEMAN_EXEMPLAR_BUILD_TESTING=OFF` as documented in upstream [CMake documentation](https://cmake.org/cmake/help/latest/module/CTest.html):

```shell
cmake -B build -S . -DBEMAN_EXEMPLAR_BUILD_TESTING=OFF
```

</details>

## Integrate beman.scope into your project

<details>
<summary> Use beman.scope directly from C++ </summary>
<!-- TODO Darius: rewrite section!-->

If you want to use `beman.scope` from your project,
you can include `beman/scope/*.hpp`  files from your C++ source files

```cpp
#include <beman/scope/identity.hpp>
```

and directly link with `libbeman.scope.a`

```shell
# Assume /opt/beman.scope staging directory.
$ c++ -o identity_usage examples/identity_usage.cpp \
    -I /opt/beman.scope/include/ \
    -L/opt/beman.scope/lib/ -lbeman.exemplar
```

</details>

<details>
<summary> Use beman.scope directly from CMake </summary>

<!-- TODO Darius: rewrite section! Add examples. -->

For CMake based projects, you will need to use the `beman.scope` CMake module to define the `beman::exemplar` CMake target:

```cmake
find_package(beman.scope REQUIRED)
```

You will also need to add `beman::scope`
to the link libraries of any libraries or executables that include `beman/scope/*.hpp` in their source or header file.

```cmake
target_link_libraries(yourlib PUBLIC beman::scope)
```

</details>

<details>
<summary> Use beman.scope from other build systems </summary>

<!-- TODO Darius: rewrite section! Add examples. -->

Build systems that support `pkg-config` by providing a `beman.scope.pc` file.
Build systems that support interoperation via `pkg-config` should be able to detect `beman.scope` for you automatically.

</details>

## Contributing

Please do! Issues and pull requests are appreciated.
