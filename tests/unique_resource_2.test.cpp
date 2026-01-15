// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "beman/scope/scope.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstdio>
#include <functional>

namespace {

struct Counter {
    int value = 0;
};

struct CountingDeleter {
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    Counter* counter{nullptr};

    void operator()(int& /*unused*/) const noexcept { ++counter->value; }
};

} // namespace

TEST_CASE("Construct file unique_resource", "[unique_resource]") {
    bool open_file_good  = false;
    bool close_file_good = false;

    {
        auto file = beman::scope::unique_resource(fopen("example.txt", "w"), // Acquire the FILE*
                                                  [&close_file_good](FILE* f) -> void {
                                                      if (f) {
                                                          (void)fclose(f); // Release (cleanup) the resource
                                                          close_file_good = true;
                                                      }
                                                  });

        if (file.get() == nullptr) {
            throw std::runtime_error("file didn't open");
        }
        open_file_good = true;
    }

    REQUIRE(open_file_good == true);
    REQUIRE(close_file_good == true);
}

TEST_CASE("unique_resource basic construction and engagement", "[unique_resource]") {
    Counter c{}; // NOLINT(misc-const-correctness)
    {
        beman::scope::unique_resource r(42, CountingDeleter{&c});

        // XXX REQUIRE(static_cast<bool>(r));
        REQUIRE(r.get() == 42);
        REQUIRE(c.value == 0);
    }

    REQUIRE(c.value == 1);
}

TEST_CASE("unique_resource release disengages without deleting", "[unique_resource]") {
    Counter c{}; // NOLINT(misc-const-correctness)
    {
        beman::scope::unique_resource r(7, CountingDeleter{&c});

        r.release();

        // XXX REQUIRE_FALSE(r);
    }

    REQUIRE(c.value == 0);
}

TEST_CASE("unique_resource reset() destroys current resource", "[unique_resource]") {
    Counter c{}; // NOLINT(misc-const-correctness)

    {
        beman::scope::unique_resource r(1, CountingDeleter{&c});

        r.reset();
        // XXX REQUIRE_FALSE(r);
        REQUIRE(c.value == 1);
    }

    REQUIRE(c.value == 1);
}

TEST_CASE("unique_resource reset(new_resource) replaces resource", "[unique_resource]") {
    Counter c{}; // NOLINT(misc-const-correctness)

    {
        beman::scope::unique_resource r(1, CountingDeleter{&c});

        r.reset(2);

        // XXX REQUIRE(r);
        REQUIRE(r.get() == 2);
        REQUIRE(c.value == 1);
    }

    REQUIRE(c.value == 2);
}

TEST_CASE("unique_resource move constructor transfers ownership", "[unique_resource]") {
    Counter c{}; // NOLINT(misc-const-correctness)

    beman::scope::unique_resource r1(10, CountingDeleter{&c});
    beman::scope::unique_resource r2(std::move(r1));

    // XXX REQUIRE_FALSE(r1);
    // XXX REQUIRE(r2);
    REQUIRE(r2.get() == 10);

    r2.reset();
    REQUIRE(c.value == 1);
}

TEST_CASE("unique_resource move assignment destroys target before transfer", "[unique_resource]") {
    Counter c1{}; // NOLINT(misc-const-correctness)
    Counter c2{}; // NOLINT(misc-const-correctness)

    beman::scope::unique_resource r1(1, CountingDeleter{&c1});
    beman::scope::unique_resource r2(2, CountingDeleter{&c2});

    r2 = std::move(r1);

    // XXX REQUIRE_FALSE(r1);
    // XXX REQUIRE(r2);
    REQUIRE(r2.get() == 1);

    REQUIRE(c2.value == 1); // old r2 destroyed
    REQUIRE(c1.value == 0);

    r2.reset();
    REQUIRE(c1.value == 1);
}

TEST_CASE("unique_resource destructor is idempotent after release", "[unique_resource]") {
    Counter c{}; // NOLINT(misc-const-correctness)

    {
        beman::scope::unique_resource r(99, CountingDeleter{&c});

        r.release();
    }

    REQUIRE(c.value == 0);
}
#ifdef BEMAN_SCOPE_USE_FALLBACK
TEST_CASE("make_unique_resource_checked disengages on invalid", "[unique_resource]") {
    Counter c{}; // NOLINT(misc-const-correctness)

    {
        auto r = beman::scope::make_unique_resource_checked(-1, -1, CountingDeleter{&c});

        // XXX REQUIRE_FALSE(r);
    }

    REQUIRE(c.value == 0);
}

TEST_CASE("make_unique_resource_checked engages on valid", "[unique_resource]") {
    Counter c{}; // NOLINT(misc-const-correctness)

    {
        auto r = beman::scope::make_unique_resource_checked(3, -1, CountingDeleter{&c});

        // XXX REQUIRE(r);
    }

    REQUIRE(c.value == 1);
}

TEST_CASE("Open a nonexisting file with make_unique_resource_checked", "[unique_resource]") {
    bool open_file_good  = false;
    bool close_file_good = false;

    {
        auto file =
            beman::scope::make_unique_resource_checked(fopen("nonexisting.txt", "r"), // Acquire the FILE*
                                                       nullptr,
                                                       [&close_file_good](FILE* f) -> void {
                                                           if (f) {
                                                               (void)fclose(f); // Release (cleanup) the resource
                                                               close_file_good = true;
                                                           }
                                                       });

        if (file.get() != nullptr) {
            open_file_good = true;
        }
    }

    REQUIRE(open_file_good == false);
    REQUIRE(close_file_good == false);
}
#endif

TEST_CASE("unique_resource supports deduction guide", "[unique_resource]") {
    Counter c{}; // NOLINT(misc-const-correctness)

    beman::scope::unique_resource r(123, CountingDeleter{&c});

    static_assert(std::is_same_v<decltype(r), beman::scope::unique_resource<int, CountingDeleter>>);

    r.reset();
    REQUIRE(c.value == 1);
}

TEST_CASE("unique_resource does not double-delete after move", "[unique_resource]") {
    Counter c{}; // NOLINT(misc-const-correctness)

    {
        beman::scope::unique_resource r1(1, CountingDeleter{&c});

        {
            auto r2 = std::move(r1);
        }

        REQUIRE(c.value == 1);
    }

    REQUIRE(c.value == 1);
}

TEST_CASE("unique_resource operator* returns reference to resource", "[unique_resource]") {
    int value = 42;

    // Define the deleter type explicitly (function pointer)
    using DeleterType = void (*)(int*);

    // Empty deleter
    auto empty_deleter = [](int*) {};

    // Create unique_resource instance (modifiable)
    beman::scope::unique_resource<int*, DeleterType> r(&value, empty_deleter);

    // operator* should return a reference
    int& ref = *r;

    // Check that the reference refers to the original value
    REQUIRE(&ref == &value);
    REQUIRE(ref == 42);

    // Modify the value through the reference
    ref = 100;
    REQUIRE(value == 100);

    // Create a const unique_resource instance
    const beman::scope::unique_resource<int*, DeleterType> r2(&value, empty_deleter);

    // operator* should return const reference
    const int& cref = *r2;
    REQUIRE(cref == 100);

    // Modifying through cref would fail to compile (correct)
}

struct Foo {
    int value = 0;
};

TEST_CASE("unique_resource operator-> works", "[unique_resource]") {
    bool deleted = false;
    Foo* raw     = new Foo{42};

    // Use std::function for the deleter
    beman::scope::unique_resource<Foo*, std::function<void(Foo*)>> r(raw, [&](Foo* p) {
        deleted = true;
        delete p;
    });

    REQUIRE(r->value == 42);
    r->value = 100;
    REQUIRE(r->value == 100);

    REQUIRE_FALSE(deleted); // deleter not called yet
}
