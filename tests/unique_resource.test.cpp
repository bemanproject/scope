#include <memory>
#include <cstdio>
#include <beman/scope/scope.hpp>

constexpr bool open_file_good = false;
constexpr bool close_file_good = false;

// define only in one cpp file
#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

TEST_CASE("Construct file unique_resource") {

  {
    auto file = beman::scope::unique_resource(
        fopen("example.txt", "w"), // Acquire the FILE*
        [](FILE* f) {
            if (f) {
                fclose(f); // Release (cleanup) the resource
                close_file_good = true;
            }
        }
    );

    if (!file.get()) {
        return 1;
    }
    open_file_good = true;
  }


  REQUIRE(open_file_good == true);
  REQUIRE(cloes_file_good == true);
}
