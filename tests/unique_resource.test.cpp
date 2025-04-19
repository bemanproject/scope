#include <memory>
#include <cstdio>
#include <stdexcept>
#include <beman/scope/scope.hpp>

// clang-format off

#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

using namespace beman::scope;
TEST_CASE("Construct file unique_resource", "[unique_resource]") {
    bool open_file_good  = false;
    bool close_file_good = false;

   {
       auto file = beman::scope::unique_resource(
           fopen("example.txt", "w"), // Acquire the FILE*
           [&close_file_good](FILE* f) { if (f) {
                            fclose(f); // Release (cleanup) the resource
                            close_file_good = true;
                         }
           } );

       if (!file.get()) {
           throw std::runtime_error("file didn't open");
       }
       open_file_good = true;
   }
   REQUIRE(open_file_good == true);
   REQUIRE(close_file_good == true);
}

struct DummyResource {
    bool* cleanedUp;

    DummyResource(bool* flag) : cleanedUp(flag) {
        *cleanedUp = false;
    }

    void do_something() const {}
};

TEST_CASE("unique_resource calls cleanup on destruction", "[unique_resource]") {
    bool cleaned = false;

    {
        auto res = unique_resource(
            DummyResource(&cleaned),
            [](DummyResource r) { *(r.cleanedUp) = true; }
        );

        res.get().do_something();
    }

    REQUIRE(cleaned == true);
}

TEST_CASE("unique_resource does not clean up after release", "[unique_resource]") {
    bool cleaned = false;

    {
        auto res = unique_resource(
            DummyResource(&cleaned),
            [](DummyResource r) { *(r.cleanedUp) = true; }
        );

        res.release(); //no cleanup run
    }

    REQUIRE(cleaned == false);
}

TEST_CASE("unique_resource moves properly", "[unique_resource]") {
    bool cleaned = false;

    unique_resource<DummyResource, void(*)(DummyResource)> res1(
        DummyResource(&cleaned),
        [](DummyResource r) { *(r.cleanedUp) = true; }
    );

    {
        auto res2 = std::move(res1);
        res2.get().do_something();
    }

    REQUIRE(cleaned == true);
}

TEST_CASE("unique_resource reset cleans up old resource", "[unique_resource]") {
    bool cleaned1 = false;
    bool cleaned2 = false;

    DummyResource res1(&cleaned1);
    DummyResource res2(&cleaned2);

    auto ur = unique_resource(
        res1,
        [](DummyResource r) { *(r.cleanedUp) = true; }
    );

    ur.reset(res2); // should clean up res1 and now manage res2

    REQUIRE(cleaned1 == true);
    REQUIRE(cleaned2 == false);
}

// Simulates throwing in the middle of a function using unique_resource
void simulate_exception(bool* cleanup_flag) {
    auto res = unique_resource(
        DummyResource(cleanup_flag),
        [](DummyResource r) { *(r.cleanedUp) = true; }
    );

    throw std::runtime_error("Something went wrong");
}

TEST_CASE("unique_resource cleans up on exception", "[unique_resource][exception]") {
    bool cleaned = false;

    try {
        simulate_exception(&cleaned);
    } catch (const std::exception& e) {
        // Expected
    }

    REQUIRE(cleaned == true);
}

TEST_CASE("unique_resource does not clean up if reset before exception", "[unique_resource][exception]") {
    bool cleaned = false;

    try {
        auto res = unique_resource(
            DummyResource(&cleaned),
            [](DummyResource r) { *(r.cleanedUp) = true; }
        );

        res.reset(); // disables cleanup, sets cleaned true

        cleaned = false; //reset cleaned

        throw std::runtime_error("Throwing after release");

    } catch (...) {
        // expected
    }

    REQUIRE(cleaned == false);
}

TEST_CASE("unique_resource handles exception during reset", "[unique_resource][exception]") {
    bool cleaned1 = false;
    bool cleaned2 = false;

    DummyResource res1(&cleaned1);
    DummyResource res2(&cleaned2);

    auto ur = unique_resource(
        res1,
        [](DummyResource r) { *(r.cleanedUp) = true; }
    );

    try {
        ur.reset(res2);
        throw std::runtime_error("Exception after reset");
    } catch (...) {
        // expected
    }

    REQUIRE(cleaned1 == true);
    REQUIRE(cleaned2 == false);
}
