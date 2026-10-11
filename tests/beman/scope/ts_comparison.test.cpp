// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Optional vendor comparisons supplement the specification-based tests. They
// cover shared behavior, not extensions or a vendor implementation's bugs.
#include <beman/scope.hpp>
#include <experimental/scope>

#include <array>
#include <stdexcept>
#include <utility>

#include <catch2/catch_all.hpp>

namespace {

template <template <class> class Exit, template <class> class Fail, template <class> class Success>
std::array<int, 3> guard_counts() {
    std::array<int, 3> counts{};
    auto               on_exit    = [&]() noexcept { ++counts[0]; };
    auto               on_fail    = [&]() noexcept { ++counts[1]; };
    auto               on_success = [&]() noexcept { ++counts[2]; };

    {
        Exit<decltype(on_exit)>       exit(on_exit);
        Fail<decltype(on_fail)>       fail(on_fail);
        Success<decltype(on_success)> success(on_success);
    }
    try {
        Exit<decltype(on_exit)>       exit(on_exit);
        Fail<decltype(on_fail)>       fail(on_fail);
        Success<decltype(on_success)> success(on_success);
        throw std::runtime_error("unwind");
    } catch (const std::runtime_error&) {
    }
    {
        Exit<decltype(on_exit)>       exit(on_exit);
        Fail<decltype(on_fail)>       fail(on_fail);
        Success<decltype(on_success)> success(on_success);
        exit.release();
        fail.release();
        success.release();
    }
    {
        Exit<decltype(on_exit)> source(on_exit);
        auto                    destination = std::move(source);
    }
    return counts;
}

template <template <class, class> class Resource>
std::array<int, 4> resource_cleanup() {
    std::array<int, 4> cleaned{};
    int                count   = 0;
    auto               deleter = [&](int handle) noexcept { cleaned[count++] = handle; };
    {
        Resource<int, decltype(deleter)> source(10, deleter);
        source.reset(20);
        auto destination = std::move(source);
        REQUIRE(destination.get() == 20);
        destination.reset();

        Resource<int, decltype(deleter)> released(30, deleter);
        released.release();
        REQUIRE(released.get() == 30);

        Resource<int, decltype(deleter)> final_resource(40, deleter);
    }
    return cleaned;
}

} // namespace

TEST_CASE("scope guards agree with the TS on shared lifetime behavior", "[ts_comparison]") {
    const auto actual =
        guard_counts<beman::scope::scope_exit, beman::scope::scope_fail, beman::scope::scope_success>();
    const auto reference =
        guard_counts<std::experimental::scope_exit, std::experimental::scope_fail, std::experimental::scope_success>();
    const std::array<int, 3> expected{3, 1, 1};
    REQUIRE(actual == expected);
    REQUIRE(reference == expected);
}

TEST_CASE("unique_resource agrees with the TS on shared ownership behavior", "[ts_comparison]") {
    const auto               actual    = resource_cleanup<beman::scope::unique_resource>();
    const auto               reference = resource_cleanup<std::experimental::unique_resource>();
    const std::array<int, 4> expected{10, 20, 40, 0};
    REQUIRE(actual == expected);
    REQUIRE(reference == expected);
}
