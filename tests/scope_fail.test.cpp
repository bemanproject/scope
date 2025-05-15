// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/scope/scope.hpp>
#include <stdexcept>
#include <string>

// clang-format off

#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

using beman::scope::scope_fail;
using beman::scope::scope_success;

TEST_CASE("scope_fail runs handler on exception", "[scope_fail]") {
    bool triggered = false;

    try {
        scope_fail guard([&]() {
            triggered = true;
        });

        throw std::runtime_error("trigger failure");

    } catch (...) {
        // expected
    }

    REQUIRE(triggered == true);
}

TEST_CASE("scope_fail does NOT run on normal scope exit", "[scope_fail]") {
    bool triggered = false;

    {
        scope_fail guard([&]() {
            triggered = true;
        });

        // no exception thrown
    }

    REQUIRE(triggered == false);
}

TEST_CASE("scope_fail can be released to cancel handler", "[scope_fail]") {
    bool triggered = false;

    try {
        scope_fail guard([&]() {
            triggered = true;
        });

        guard.release();

        throw std::runtime_error("fail");

    } catch (...) {
        // expected
    }

    REQUIRE(triggered == false);
}

TEST_CASE("scope_fail supports move-only captures", "[scope_fail]") {
    bool released = false;

    try {
        auto ptr = std::make_unique<int>(99);

        scope_fail guard([p = std::move(ptr), &released]() {
            released = (p != nullptr);
        });

        throw std::runtime_error("fail");

    } catch (...) {
        // expected
    }

    REQUIRE(released == true);
}

TEST_CASE("scope_fail handles nested guards in reverse order", "[scope_fail]") {
    std::string trace;

    try {
        scope_fail g1([&] { trace += "first "; });
        scope_fail g2([&] { trace += "second "; });

        throw std::runtime_error("fail");

    } catch (...) {}

    REQUIRE(trace == "second first ");
}

TEST_CASE("scope_fail handler can include side-effect cleanup", "[scope_fail]") {
    struct Tracker {
        std::string& output;
        ~Tracker() { output += " destroyed"; }
    };

    std::string result;

    try {
        Tracker t{result};

        scope_fail guard([&]() {
            result += " failure";
        });

        throw std::runtime_error("fail");

    } catch (...) {}

    REQUIRE(result == " failure destroyed");
}


TEST_CASE("scope_fail handler works inside noexcept lambda (not triggered)", "[scope_fail][advanced]") {
    bool triggered = false;

    auto test = [&]() noexcept {
        scope_fail guard([&]() {
            triggered = true;
        });

        // Can't throw inside noexcept
    };

    test();

    REQUIRE(triggered == false); // handler should not run
}

TEST_CASE("scope_fail does not trigger on conditional early return", "[scope_fail][advanced]") {
    bool triggered = false;

    auto test = [&]() {
        scope_fail guard([&]() {
            triggered = true;
        });

        if (true) return;
    };

    test();

    REQUIRE(triggered == false); // no exception = no handler
}

TEST_CASE("scope_fail with move-only captures", "[scope_fail][advanced]") {
    bool captured = false;

    try {
        auto ptr = std::make_unique<int>(5);

        scope_fail guard([p = std::move(ptr), &captured]() {
            captured = (p != nullptr);
        });

        throw std::runtime_error("test");

    } catch (...) {}

    REQUIRE(captured == true);
}

TEST_CASE("scope_fail supports release just before exception", "[scope_fail][advanced]") {
    bool triggered = false;

    try {
        scope_fail guard([&]() {
            triggered = true;
        });

        guard.release();

        throw std::runtime_error("fail");

    } catch (...) {}

    REQUIRE(triggered == false);
}

TEST_CASE("scope_fail coexists with scope_success", "[scope_fail][advanced]") {
    std::string output;

    try {
        scope_success success([&]() {
            output += "success ";
        });

        scope_fail fail([&]() {
            output += "fail ";
        });

        throw std::runtime_error("fail");

    } catch (...) {}

    REQUIRE(output == "fail "); // only fail triggered
}

TEST_CASE("scope_fail executes multiple guards in reverse order", "[scope_fail][advanced]") {
    std::string trace;

    try {
        scope_fail g1([&] { trace += "first "; });
        scope_fail g2([&] { trace += "second "; });
        scope_fail g3([&] { trace += "third "; });

        throw std::runtime_error("fail");

    } catch (...) {}

    REQUIRE(trace == "third second first ");
}

TEST_CASE("scope_fail doesn't trigger in function that catches its own exception", "[scope_fail][advanced]") {
    bool triggered = false;

    auto test = [&]() {
        scope_fail guard([&]() {
            triggered = true;
        });

        try {
            throw std::runtime_error("internal");
        } catch (...) {
            // handled internally
        }
    };

    test();

    REQUIRE(triggered == false);
}

TEST_CASE("scope_fail behaves correctly when moved", "[scope_fail][advanced]") {
    bool triggered = false;

    try {
        scope_fail g1([&]() { triggered = true; });
        auto g2 = std::move(g1);  // ownership transferred

        throw std::runtime_error("fail");

    } catch (...) {}

    REQUIRE(triggered == true);
}
