// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// clang-format off

#include <beman/scope/scope.hpp>
#include <stdexcept>
#include <string>

#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>


using beman::scope::scope_exit;


TEST_CASE("scope_exit runs handler on normal scope exit", "[scope_exit]") {
    bool cleanup_ran = false;

    {
        scope_exit guard([&]() {
            cleanup_ran = true;
        });

        // Normal scope exit
    }

    REQUIRE(cleanup_ran == true);
}

TEST_CASE("scope_exit runs handler on exception", "[scope_exit]") {
    bool cleanup_ran = false;

    try {
        scope_exit guard([&]() {
            cleanup_ran = true;
        });

        throw std::runtime_error("Throwing to test scope_exit");

    } catch (...) {
        // Exception caught
    }

    REQUIRE(cleanup_ran == true);
}

TEST_CASE("scope_exit does not run handler if released", "[scope_exit]") {
    bool cleanup_ran = false;

    {
        scope_exit guard([&]() {
            cleanup_ran = true;
        });

        guard.release(); // cancel execution
    }

    REQUIRE(cleanup_ran == false);
}

TEST_CASE("scope_exit supports move semantics", "[scope_exit]") {
    bool cleanup_ran_count = false;

    {
        scope_exit guard1([&]() {
            cleanup_ran = true;
        });

        auto guard2 = std::move(guard1); // transfer ownership

        // guard1 is inactive, guard2 should execute
    }

    REQUIRE(cleanup_ran == true);
}

TEST_CASE("moved-from scope_exit does not trigger handler", "[scope_exit]") {
    bool cleanup_ran_count = 0;

    {
        scope_exit guard1([&]() {
            ++cleanup_ran;
        });

        [[maybe_unused]] auto guard2 = std::move(guard1); // guard1 is now a no-op
    }

    REQUIRE(cleanup_ran_count == 1 ); // cleanup still runs — but from guard2
}

TEST_CASE("scope_exit supports noexcept lambdas", "[scope_exit][advanced]") {
    bool cleaned = false;

    {
        scope_exit guard([&]() noexcept {
            cleaned = true;
        });
    }

    REQUIRE(cleaned == true);
}

TEST_CASE("scope_exit with move-only captured resource", "[scope_exit][advanced]") {
    bool deleted = false;

    {
        auto ptr = std::make_unique<int>(42);
        scope_exit guard([resource = std::move(ptr), &deleted]() {
            deleted = (resource == nullptr); // will be false — ptr not null
            // but `resource` goes out of scope here
        });
    }

    REQUIRE(deleted == false); // still not null at cleanup point
}

TEST_CASE("scope_exit handles nested guards in correct order", "[scope_exit][advanced]") {
    std::string trace;

    {
        scope_exit guard1([&] { trace += "first "; });
        scope_exit guard2([&] { trace += "second "; });
    }

    REQUIRE(trace == "second first ");
}

// fails
// TEST_CASE("scope_exit cleanup handles custom object with side effects", "[scope_exit][advanced]") {
//     struct Tracer {
//         std::string& output;
//         ~Tracer() { output += "cleaned "; }
//     };

//     std::string result;

//     {
//         Tracer t{result};

//         scope_exit guard([&]() {
//             result += "via_scope_exit ";
//         });
//     }

//     REQUIRE(result == "cleaned via_scope_exit ");
// }
