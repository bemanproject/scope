// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

import beman.scope;
// #include <stdexcept>
// #include <string>
#include <cassert>

// #define CATCH_CONFIG_MAIN
// #include <catch2/catch_all.hpp>

// clang-format off
// struct noisy_resource {
//     noisy_resource()  { std::print( "construct noisy\n" ); }
//     ~noisy_resource() { std::print( "destroy noisy\n"   ); }
// };

//TEST_CASE("module-test", "[scope_module_test]") {
int main()
{
    bool exit_ran, success_ran, fail_ran = false;
    {
        beman::scope::scope_exit    _([&exit_ran]    { exit_ran = true;    });
        beman::scope::scope_success _([&success_ran] { success_ran = true; });
        beman::scope::scope_fail    _([&fail_ran]    { fail_ran = true;    });
        // auto                        resource_ptr = beman::scope::unique_resource(new noisy_resource(),
        //                                                   // Cleanup function
        //                                                   [](noisy_resource* ptr) { delete ptr; });
    } // Normal scope exit

    assert(exit_ran == true);
    assert(success_ran == true);
    assert(fail_ran == false);

 }
