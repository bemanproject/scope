<!-- SPDX-License-Identifier: CC0-1.0 -->

# Resources

This file contains a variety of links to prior art in this area including related libraries and videos.

## Papers

- TS design and wording paper [p0052 - Generic Scope Guard and RAII Wrapper for the Standard Library](https://wg21.link/p0052)
- TS adoption paper [p1411 - Please reconsider <scope> for C++20](https://wg21.link/p1411)
- [N3677 A Proposal to Add additional RAII Wrappers to the Standard Library](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3677.html)
- [N4152 uncaught_exceptions - Sutter](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4152.pdf)

## Implementations

- [TS example of scope_exit](https://godbolt.org/z/T5KhTYjP7)
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
