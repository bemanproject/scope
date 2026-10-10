// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/scope/unique_resource.hpp>

#include <array>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <catch2/catch_test_macros.hpp>

using beman::scope::make_unique_resource_checked;
using beman::scope::unique_resource;

namespace {

struct cleanup_trace {
    std::array<int, 16> calls{};
};

struct faults {
    bool construct = false;
    bool assign    = false;
    int  copies    = 0;
};

// Moving may throw, so the wrapper must copy. Failures happen inside storage
// operations rather than after a completed reset or move.
struct copy_resource {
    int     value;
    faults* control;

    copy_resource(int v, faults& f) : value(v), control(&f) {}
    copy_resource(const copy_resource& rhs) : value(rhs.value), control(rhs.control) {
        ++control->copies;
        if (control->construct) {
            throw std::runtime_error("resource copy failed");
        }
    }
    copy_resource(copy_resource&&) noexcept(false) {
        throw std::runtime_error("throwing resource move must not be selected");
    }
    copy_resource& operator=(const copy_resource& rhs) {
        if (rhs.control->assign) {
            throw std::runtime_error("resource assignment failed");
        }
        value   = rhs.value;
        control = rhs.control;
        return *this;
    }
    copy_resource& operator=(copy_resource&&) noexcept(false) {
        throw std::runtime_error("throwing resource assignment must not be selected");
    }
};

struct move_resource {
    int value;
    explicit move_resource(int v) : value(v) {}
    move_resource(move_resource&& rhs) noexcept : value(std::exchange(rhs.value, -1)) {}
    move_resource& operator=(move_resource&& rhs) noexcept {
        value = std::exchange(rhs.value, -1);
        return *this;
    }
};

struct counting_deleter {
    cleanup_trace* trace = nullptr;
    void           operator()(int value) const noexcept { ++trace->calls.at(static_cast<std::size_t>(value)); }
    void           operator()(const copy_resource& r) const noexcept { (*this)(r.value); }
    void           operator()(const move_resource& r) const noexcept { (*this)(r.value); }
};

struct copy_deleter : counting_deleter {
    faults* control;
    copy_deleter(cleanup_trace& trace, faults& f) : counting_deleter{&trace}, control(&f) {}
    copy_deleter(const copy_deleter& rhs) : counting_deleter(rhs), control(rhs.control) {
        ++control->copies;
        if (control->construct) {
            throw std::runtime_error("deleter copy failed");
        }
    }
    copy_deleter(copy_deleter&&) noexcept(false) {
        throw std::runtime_error("throwing deleter move must not be selected");
    }
    copy_deleter& operator=(const copy_deleter& rhs) {
        if (rhs.control->assign) {
            throw std::runtime_error("deleter assignment failed");
        }
        trace   = rhs.trace;
        control = rhs.control;
        return *this;
    }
    copy_deleter& operator=(copy_deleter&&) noexcept(false) {
        throw std::runtime_error("throwing deleter assignment must not be selected");
    }
};

struct constexpr_deleter {
    int*           cleaned = nullptr;
    constexpr void operator()(int value) const noexcept { *cleaned += value; }
};

constexpr bool constexpr_ownership() {
    int cleaned = 0;
    {
        auto source = make_unique_resource_checked(2, -1, constexpr_deleter{&cleaned});
        auto target = std::move(source);
        target.reset(3);
        source.reset();
    }
    auto invalid = make_unique_resource_checked(-1, -1, constexpr_deleter{&cleaned});
    invalid.reset();
    return cleaned == 5 && invalid.get() == -1;
}

template <class T>
concept dereferenceable = requires(const T& r) { *r; };

template <class T>
concept arrow_accessible = requires(const T& r) { r.operator->(); };

static_assert(constexpr_ownership());
static_assert(!std::is_copy_constructible_v<unique_resource<int, counting_deleter>>);
static_assert(!std::is_copy_assignable_v<unique_resource<int, counting_deleter>>);
static_assert(std::is_nothrow_move_constructible_v<unique_resource<int, counting_deleter>>);
static_assert(std::is_nothrow_move_assignable_v<unique_resource<int, counting_deleter>>);
static_assert(!std::is_nothrow_constructible_v<unique_resource<int, copy_deleter>, int, copy_deleter&>);
static_assert(!std::is_nothrow_move_constructible_v<unique_resource<copy_resource, counting_deleter>>);
static_assert(!std::is_nothrow_move_assignable_v<unique_resource<int, copy_deleter>>);
static_assert(!std::is_default_constructible_v<unique_resource<int&, counting_deleter>>);
static_assert(dereferenceable<unique_resource<int*, counting_deleter>>);
static_assert(!dereferenceable<unique_resource<void*, counting_deleter>>);
static_assert(!dereferenceable<unique_resource<int, counting_deleter>>);
static_assert(arrow_accessible<unique_resource<void*, counting_deleter>>);
static_assert(!arrow_accessible<unique_resource<int, counting_deleter>>);
static_assert(
    std::is_same_v<decltype(std::declval<const unique_resource<int, counting_deleter>&>().get()), const int&>);
static_assert(std::is_same_v<decltype(std::declval<const unique_resource<int&, counting_deleter>&>().get()), int&>);
static_assert(std::is_same_v<decltype(std::declval<const unique_resource<int, counting_deleter>&>().get_deleter()),
                             const counting_deleter&>);

} // namespace

TEST_CASE("default unique_resource is value initialized and inactive", "[unique_resource]") {
    unique_resource<int, counting_deleter> r;
    REQUIRE(r.get() == 0);
    REQUIRE(r.get_deleter().trace == nullptr);
    r.reset();
}

TEST_CASE("unique_resource cleanup is exactly once across resets and destruction", "[unique_resource]") {
    cleanup_trace trace;
    {
        unique_resource r(1, counting_deleter{&trace});
        r.reset();
        r.reset();
        REQUIRE(trace.calls[1] == 1);
        r.reset(2);
        REQUIRE(r.get() == 2);
    }
    REQUIRE(trace.calls[1] == 1);
    REQUIRE(trace.calls[2] == 1);
}

TEST_CASE("moving a released unique_resource does not reactivate it", "[unique_resource]") {
    cleanup_trace trace;
    {
        unique_resource source(1, counting_deleter{&trace});
        source.release();
        auto target = std::move(source);
        REQUIRE(target.get() == 1);
    }
    REQUIRE(trace.calls[1] == 0);
}

TEST_CASE("move assignment cleans up the destination with its original deleter", "[unique_resource]") {
    cleanup_trace source_trace;
    cleanup_trace target_trace;
    {
        unique_resource source(1, counting_deleter{&source_trace});
        unique_resource target(2, counting_deleter{&target_trace});
        target = std::move(source);
        REQUIRE(target.get() == 1);
        REQUIRE(target.get_deleter().trace == &source_trace);
        REQUIRE(target_trace.calls[2] == 1);
        source.reset();
        REQUIRE(source_trace.calls[1] == 0);
    }
    REQUIRE(source_trace.calls[1] == 1);
    REQUIRE(target_trace.calls[2] == 1);
}

TEST_CASE("move assignment from a released wrapper leaves the destination inactive", "[unique_resource]") {
    cleanup_trace trace;
    {
        unique_resource source(1, counting_deleter{&trace});
        unique_resource target(2, counting_deleter{&trace});
        source.release();
        target = std::move(source);
        REQUIRE(target.get() == 1);
        REQUIRE(trace.calls[2] == 1);
    }
    REQUIRE(trace.calls[1] == 0);
    REQUIRE(trace.calls[2] == 1);
}

TEST_CASE("self move assignment preserves resource ownership", "[unique_resource]") {
    cleanup_trace trace;
    {
        unique_resource r(1, counting_deleter{&trace});
        auto&           same = r;
        r                    = std::move(same);
        REQUIRE(r.get() == 1);
        REQUIRE(trace.calls[1] == 0);
    }
    REQUIRE(trace.calls[1] == 1);
}

TEST_CASE("checked factory preserves valid and invalid handles", "[unique_resource]") {
    cleanup_trace trace;
    {
        auto valid   = make_unique_resource_checked(3, -1, counting_deleter{&trace});
        auto invalid = make_unique_resource_checked(4, 4, counting_deleter{&trace});
        REQUIRE(valid.get() == 3);
        REQUIRE(invalid.get() == 4);
        invalid.reset();
    }
    REQUIRE(trace.calls[0] == 0);
    REQUIRE(trace.calls[3] == 1);
    REQUIRE(trace.calls[4] == 0);
}

TEST_CASE("checked factory accepts nondefault move-only resources", "[unique_resource]") {
    cleanup_trace trace;
    struct invalid_value {};
    struct handle : move_resource {
        using move_resource::move_resource;
        bool operator==(invalid_value) const noexcept { return value == -1; }
    };
    {
        auto r = make_unique_resource_checked(handle{3}, invalid_value{}, counting_deleter{&trace});
        REQUIRE(r.get().value == 3);
    }
    REQUIRE(trace.calls[3] == 1);
}

TEST_CASE("resource references remain references and reset rebinds them", "[unique_resource]") {
    cleanup_trace trace;
    int           first  = 1;
    int           second = 2;
    {
        unique_resource<int&, counting_deleter> r(first, counting_deleter{&trace});
        REQUIRE(&r.get() == &first);
        r.reset(second);
        REQUIRE(trace.calls[1] == 1);
        REQUIRE(first == 1);
        REQUIRE(&r.get() == &second);
        auto moved = std::move(r);
        REQUIRE(&moved.get() == &second);
    }
    REQUIRE(trace.calls[1] == 1);
    REQUIRE(trace.calls[2] == 1);
}

TEST_CASE("pointer observers access the pointee through a const wrapper", "[unique_resource]") {
    struct resource {
        int value;
    } object{3};
    int cleaned = 0;
    {
        const unique_resource r(&object, [&](resource* p) noexcept {
            REQUIRE(p == &object);
            ++cleaned;
        });
        REQUIRE(&*r == &object);
        r->value = 4;
        REQUIRE(object.value == 4);
    }
    REQUIRE(cleaned == 1);
}

TEST_CASE("unique_resource accepts a move-only deleter", "[unique_resource]") {
    struct deleter {
        int* calls;
        explicit deleter(int& c) : calls(&c) {}
        deleter(deleter&& rhs) noexcept : calls(std::exchange(rhs.calls, nullptr)) {}
        void operator()(int) noexcept { ++*calls; }
    };
    int cleaned = 0;
    {
        unique_resource r(1, deleter{cleaned});
        auto            moved = std::move(r);
    }
    REQUIRE(cleaned == 1);
}

TEST_CASE("failed resource construction cleans up the input once", "[unique_resource][exception]") {
    cleanup_trace trace;
    faults        control;
    copy_resource input(1, control);
    control.construct = true;
    REQUIRE_THROWS_AS((unique_resource(input, counting_deleter{&trace})), std::runtime_error);
    REQUIRE(trace.calls[1] == 1);
}

TEST_CASE("failed deleter construction cleans up the stored resource once", "[unique_resource][exception]") {
    cleanup_trace trace;
    faults        control;
    copy_deleter  d(trace, control);
    move_resource input(1);
    control.construct = true;
    REQUIRE_THROWS_AS((unique_resource(std::move(input), d)), std::runtime_error);
    REQUIRE(input.value == -1);
    REQUIRE(trace.calls[1] == 1);
}

TEST_CASE("checked factory failure cleans up only valid input", "[unique_resource][exception]") {
    cleanup_trace trace;
    faults        control;
    copy_deleter  d(trace, control);
    control.construct = true;
    REQUIRE_THROWS_AS(make_unique_resource_checked(1, -1, d), std::runtime_error);
    REQUIRE(trace.calls[1] == 1);
    REQUIRE_THROWS_AS(make_unique_resource_checked(2, 2, d), std::runtime_error);
    REQUIRE(trace.calls[2] == 0);
}

TEST_CASE("throwing moves use the copy fallback", "[unique_resource]") {
    cleanup_trace trace;
    faults        resource_control;
    faults        deleter_control;
    copy_resource input(1, resource_control);
    copy_deleter  d(trace, deleter_control);
    {
        unique_resource r(std::move(input), std::move(d));
        auto            moved = std::move(r);
        REQUIRE(moved.get().value == 1);
        REQUIRE(resource_control.copies == 2);
        REQUIRE(deleter_control.copies == 2);
    }
    REQUIRE(trace.calls[1] == 1);
}

TEST_CASE("failed resource copy during move leaves source owning", "[unique_resource][exception]") {
    cleanup_trace trace;
    faults        control;
    copy_resource input(1, control);
    {
        unique_resource source(input, counting_deleter{&trace});
        control.construct = true;
        REQUIRE_THROWS_AS((unique_resource(std::move(source))), std::runtime_error);
        REQUIRE(source.get().value == 1);
        REQUIRE(trace.calls[1] == 0);
    }
    REQUIRE(trace.calls[1] == 1);
}

TEST_CASE("failed deleter copy after resource move releases the source", "[unique_resource][exception]") {
    cleanup_trace trace;
    faults        control;
    copy_deleter  d(trace, control);
    {
        unique_resource source(move_resource{1}, d);
        control.construct = true;
        REQUIRE_THROWS_AS((unique_resource(std::move(source))), std::runtime_error);
        REQUIRE(trace.calls[1] == 1);
        source.reset();
    }
    REQUIRE(trace.calls[1] == 1);
}

TEST_CASE("failed deleter copy after resource copy leaves source owning", "[unique_resource][exception]") {
    cleanup_trace trace;
    faults        resource_control;
    faults        deleter_control;
    copy_deleter  d(trace, deleter_control);
    {
        unique_resource source(copy_resource{1, resource_control}, d);
        deleter_control.construct = true;
        REQUIRE_THROWS_AS((unique_resource(std::move(source))), std::runtime_error);
        REQUIRE(trace.calls[1] == 0);
    }
    REQUIRE(trace.calls[1] == 1);
}

TEST_CASE("failed move of a released wrapper never cleans up the handle", "[unique_resource][exception]") {
    cleanup_trace trace;
    faults        control;
    copy_deleter  d(trace, control);
    {
        unique_resource source(move_resource{1}, d);
        source.release();
        control.construct = true;
        REQUIRE_THROWS_AS((unique_resource(std::move(source))), std::runtime_error);
    }
    REQUIRE(trace.calls[1] == 0);
}

TEST_CASE("failed replacement cleans up old and incoming handles once", "[unique_resource][exception]") {
    cleanup_trace trace;
    faults        control;
    copy_resource old_handle(1, control);
    copy_resource new_handle(2, control);
    {
        unique_resource r(old_handle, counting_deleter{&trace});
        control.assign = true;
        REQUIRE_THROWS_AS(r.reset(new_handle), std::runtime_error);
        REQUIRE(trace.calls[1] == 1);
        REQUIRE(trace.calls[2] == 1);
        r.reset();
    }
    REQUIRE(trace.calls[1] == 1);
    REQUIRE(trace.calls[2] == 1);
}

TEST_CASE("failed deleter assignment leaves source owning and target inactive", "[unique_resource][exception]") {
    cleanup_trace source_trace;
    cleanup_trace target_trace;
    faults        source_control;
    faults        target_control;
    copy_deleter  source_deleter(source_trace, source_control);
    copy_deleter  target_deleter(target_trace, target_control);
    {
        unique_resource source(move_resource{1}, source_deleter);
        unique_resource target(move_resource{2}, target_deleter);
        source_control.assign = true;
        REQUIRE_THROWS_AS(target = std::move(source), std::runtime_error);
        REQUIRE(source.get().value == 1);
        REQUIRE(source_trace.calls[1] == 0);
        REQUIRE(target_trace.calls[2] == 1);
        target.reset();
    }
    REQUIRE(source_trace.calls[1] == 1);
    REQUIRE(target_trace.calls[2] == 1);
}

TEST_CASE("failed resource assignment leaves source owning and target inactive", "[unique_resource][exception]") {
    cleanup_trace trace;
    faults        source_control;
    faults        target_control;
    {
        unique_resource source(copy_resource{1, source_control}, counting_deleter{&trace});
        unique_resource target(copy_resource{2, target_control}, counting_deleter{&trace});
        source_control.assign = true;
        REQUIRE_THROWS_AS(target = std::move(source), std::runtime_error);
        REQUIRE(source.get().value == 1);
        REQUIRE(trace.calls[1] == 0);
        REQUIRE(trace.calls[2] == 1);
        target.reset();
    }
    REQUIRE(trace.calls[1] == 1);
    REQUIRE(trace.calls[2] == 1);
}
