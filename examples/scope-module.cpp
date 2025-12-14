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

// #ifdef HAS_MODULE_STD
import std;
// #else
// NOTE: this needs C++23!
// #include <print>
// #endif

import beman.scope;

// clang-format off
struct noisy_resource {
    noisy_resource()  { std::print( "construct noisy\n" ); }
    ~noisy_resource() { std::print( "destroy noisy\n"   ); }
};

int main() {

    bool exit_ran, success_ran, fail_ran = false;
    {
        std::print("--> scope start\n");
        beman::scope::scope_exit    _([&exit_ran]    { exit_ran = true;    });
        beman::scope::scope_success _([&success_ran] { success_ran = true; });
        beman::scope::scope_fail    _([&fail_ran]    { fail_ran = true;    });
        auto                        resource_ptr = beman::scope::unique_resource(new noisy_resource(),
                                                          // Cleanup function
                                                          [](noisy_resource* ptr) { delete ptr; });
        std::print("--> scope end\n");
    } // Normal scope exit

    std::print("scope exit: {} success: {} fail: {} \n", exit_ran, success_ran, fail_ran);
}
