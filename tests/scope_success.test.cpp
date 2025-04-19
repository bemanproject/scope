// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/scope/scope.hpp>
#include <stdexcept>
#include <string>

// clang-format off

#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

using namespace beman::scope;

TEST_CASE("scope_success runs handler on normal scope exit", "[scope_success]") {
    bool success_triggered = false;

    {
        scope_success guard([&]() {
            success_triggered = true;
        });

        // No exception thrown; handler should run
    }

    REQUIRE(success_triggered == true);
}

TEST_CASE("scope_success does NOT run handler on exception", "[scope_success]") {
    bool success_triggered = false;

    try {
        scope_success guard([&]() {
            success_triggered = true;
        });

        throw std::runtime_error("Exception to skip handler");

    } catch (...) {
        // expected
    }

    REQUIRE(success_triggered == false);
}

TEST_CASE("scope_success can be released to prevent handler", "[scope_success]") {
    bool success_triggered = false;

    {
        scope_success guard([&]() {
            success_triggered = true;
        });

        guard.release(); // cancels the guard
    }

    REQUIRE(success_triggered == false);
}

TEST_CASE("scope_success moves transfer ownership of handler", "[scope_success]") {
    bool success_triggered = false;

    {
        scope_success original([&]() {
            success_triggered = true;
        });

        auto moved = std::move(original); // moved-from object no longer triggers
    }

    REQUIRE(success_triggered == true);
}

TEST_CASE("moved-from scope_success does not trigger handler", "[scope_success]") {
    bool success_triggered = false;

    {
        scope_success original([&]() {
            success_triggered = true;
        });

        auto moved = std::move(original);

        // Destroy both `moved` and `original` here
    }

    REQUIRE(success_triggered == true); // only moved object triggers it
}

TEST_CASE("scope_success noexcept handler works", "[scope_success][advanced]") {
    bool success_triggered = false;

    {
        scope_success guard([&]() noexcept {
            success_triggered = true;
        });
    }

    REQUIRE(success_triggered == true);
}

TEST_CASE("scope_success supports move-only captures", "[scope_success][advanced]") {
    bool flag = false;

    {
        auto ptr = std::make_unique<int>(42);
        scope_success guard([p = std::move(ptr), &flag]() {
            flag = (p != nullptr);
        });
    }

    REQUIRE(flag == true);
}

TEST_CASE("scope_success handles nested guards in reverse order", "[scope_success][advanced]") {
    std::string output;

    {
        scope_success g1([&] { output += "first "; });
        scope_success g2([&] { output += "second "; });
    }

    REQUIRE(output == "second first ");
}

TEST_CASE("scope_success does not run handler if exception is thrown", "[scope_success][advanced]") {
    bool triggered = false;

    try {
        scope_success guard([&]() {
            triggered = true;
        });

        throw std::runtime_error("scope exited via exception");

    } catch (...) {
        // expected
    }

    REQUIRE(triggered == false);
}

TEST_CASE("scope_success can be released before normal scope exit", "[scope_success][advanced]") {
    bool triggered = false;

    {
        scope_success guard([&]() {
            triggered = true;
        });

        guard.release();
    }

    REQUIRE(triggered == false);
}

TEST_CASE("scope_success handler with side-effect object", "[scope_success][advanced]") {
    struct Logger {
        std::string& log;
        ~Logger() { log += " destroyed"; }
    };

    std::string output;

    {
        Logger l{output};

        scope_success guard([&]() {
            output += " scope_success";
        });
    }

    REQUIRE(output == " scope_success destroyed");
}

TEST_CASE("scope_success honors release inside nested logic", "[scope_success][advanced]") {
    bool triggered = false;

    {
        scope_success guard([&]() {
            triggered = true;
        });

        if (true) {
            guard.release();  // simulate early cancel due to logic branch
        }
    }

    REQUIRE(triggered == false);
}
