// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

// for g++-15 the order is important -- import after includes
import beman.scope;


struct DummyResource {
    bool& cleaned;

    DummyResource(bool& flag) : cleaned(flag) { cleaned = false; }

    bool is_clean() const { return cleaned; }
};

TEST_CASE("module-test", "[scope_module_test]") {
    bool exit_ran, success_ran, fail_ran = false;
    bool cleaned = true;
    {
        // clang-format off
        beman::scope::scope_exit    _se([&exit_ran]    { exit_ran = true;    });
        beman::scope::scope_success _ss([&success_ran] { success_ran = true; });
        beman::scope::scope_fail    _sf([&fail_ran]    { fail_ran = true;    });
        auto                        resource_ptr = beman::scope::unique_resource(new DummyResource(cleaned),
										 [](DummyResource* ptr) { ptr->cleaned =true; delete ptr; });
	REQUIRE(cleaned == false);
	REQUIRE(resource_ptr->is_clean() == false);
        //  clang-format on
    } // Normal scope exit

    REQUIRE(exit_ran == true);
    REQUIRE(success_ran == true);
    REQUIRE(fail_ran == false);
    REQUIRE(cleaned == true);

}
