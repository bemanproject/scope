// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// This example uses c++20 modules with beman.scope
//
// The following are by hand instructions for compiling with g++-15
// first line generates gcm.cache file for standard headers - one time only
// g++-15 -std=c++26 -O2 -fmodules -fsearch-include-path -fmodule-only -c bits/std.cc
// g++-15 -std=c++26 -O2 -fmodules -fmodule-only -c ${scope_top}/include/beman/scope/beman.scope.cppm
// g++-15 -std=c++26 -fmodules scope-module.cpp
//
// prints:
// --> scope start
// construct noisy
// --> scope end
// destroy noisy
// scope exit: true success: true fail: false

#include <cassert>

// NOTE: this needs C++23! CK
#ifdef BEMAN_SCOPE_IMPORT_STD
import std;
#endif

// for g++-15 the order is important -- import after #includes
import beman.scope;

namespace {

struct DummyResource {
    bool& cleaned;

    DummyResource(bool& flag) : cleaned(flag) { cleaned = false; }

    [[nodiscard]] bool is_clean() const { return cleaned; }
};

} // namespace

int main() {

    bool exit_ran{};
    bool success_ran{};
    bool fail_ran{};
    bool cleaned{true};
    {
        // clang-format off
        beman::scope::scope_exit    _se([&exit_ran]    { exit_ran = true;    });
        beman::scope::scope_success _ss([&success_ran] { success_ran = true; });
        beman::scope::scope_fail    _sf([&fail_ran]    { fail_ran = true;    });
        auto resource_ptr = beman::scope::unique_resource(new DummyResource(cleaned),
                [](DummyResource* ptr) { ptr->cleaned = true; delete ptr; });
        //  clang-format on

        assert(cleaned == false);
        assert(resource_ptr->is_clean() == false);
    } // Normal scope exit

    assert(exit_ran == true);
    assert(success_ran == true);
    assert(fail_ran == false);
    assert(cleaned == true);
}
// clang-format on
