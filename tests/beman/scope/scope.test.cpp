// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/scope/scope.hpp>

// define only in one cpp file
#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

// if this compiled and no exceptions are thrown, below tests will complete successfully

// This is testing the "language" - the ways in which we want to "start" the scope guard.

TEST_CASE("Construct scope_exit") {

  // beman::scope::scope_exit guard_exit1([]) {});
  // beman::scope::scope_exit guard_exit2 = [] {};
  // beman::scope::scope_exit guard_exit3 = {[] {}};

    auto guard_exit4 = beman::scope::scope_exit([]() {});

  REQUIRE(true);
}

TEST_CASE("Construct scope_fail") {

    // beman::scope::scope_fail guard_exit1([] {});
    // beman::scope::scope_fail guard_exit2 = [] {};
    // beman::scope::scope_fail guard_exit3 = {[] {}};

  auto guard_exit4 = beman::scope::scope_fail([](){});

  REQUIRE(true);
}

TEST_CASE("Construct scope_success") {

    // beman::scope::scope_success guard_exit1([] {});
    // beman::scope::scope_success guard_exit2 = [] {};
    // beman::scope::scope_success guard_exit3 = {[] {}};

  auto guard_exit4 = beman::scope::scope_success([]() {});

  REQUIRE(true);
}
