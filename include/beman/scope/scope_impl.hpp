// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef SCOPE_IMPL_HPP
#define SCOPE_IMPL_HPP

// clang-format off
#include <version>

#if defined(__cpp_concepts) && __cpp_concepts >= 201907L
  // C++20 concepts supported
#elif __cplusplus < 202002L
#  error "C++20 or later is required"
#endif

// detect standard header first, then experimental, otherwise use local implementation
#ifdef __has_include
#  if __has_include(<scope>)
#    include <scope>
#    define BEMAN_SCOPE_USE_STD
#  elif __has_include(<experimental/scope>)
#    include <experimental/scope>
#    define BEMAN_SCOPE_USE_STD_EXPERIMENTAL
#  else
#    define BEMAN_SCOPE_USE_FALLBACK
#  endif
#else
#  define BEMAN_SCOPE_USE_FALLBACK
#endif

#ifdef BEMAN_SCOPE_USE_STD
#  if !defined(__cpp_lib_scope_exit)
#    error "Standard <scope> present but __cpp_lib_scope_exit not defined"
#  endif
#endif
// clang-format on

#ifdef BEMAN_SCOPE_USE_FALLBACK
#include <exception>
#include <type_traits>
#include <utility>

namespace beman::scope {

// TODO(CK): make a std::experimental::scope_exit<EF>::scope_exit conform
// implementation
template <class F>
class [[nodiscard]] scope_exit {
    F    f;
    bool active = true;

  public:
    constexpr explicit scope_exit(F func) noexcept(std::is_nothrow_move_constructible_v<F>) : f(std::move(func)) {}

    // Move constructor
    constexpr scope_exit(scope_exit&& other) noexcept(std::is_nothrow_move_constructible_v<F>)
        : f(std::move(other.f)), active(other.active) {
        other.active = false;
    }

    // Deleted copy
    auto operator=(const scope_exit&) -> scope_exit& = delete;
    scope_exit(const scope_exit&)                    = delete;

    // Move assignment
    constexpr auto operator=(scope_exit&& other) noexcept(std::is_nothrow_move_assignable_v<F>) -> scope_exit& {
        if (this != &other) {
            f            = std::move(other.f);
            active       = other.active;
            other.active = false;
        }
        return *this;
    }

    // Destructor: call only if scope is exiting normally
    ~scope_exit() noexcept(noexcept(f())) {
        if (active) {
            f();
        }
    }

    // Release to prevent execution
    constexpr auto release() -> void { active = false; }
};

// Factory helper
template <class F>
static auto make_scope_exit(F f) -> scope_exit<F> {
    return scope_exit<F>(std::move(f));
}

// TODO(CK): make a std::experimental::scope_fail<EF>::scope_fail conform
// implementation
template <typename F>
class [[nodiscard]] scope_fail {
    F    f;
    bool active = true;
    int  exception_count{};

  public:
    // Constructor: capture current uncaught exceptions
    constexpr explicit scope_fail(F func) noexcept(std::is_nothrow_move_constructible_v<F>)
        : f(std::move(func)), exception_count(std::uncaught_exceptions()) {}

    // Move constructor
    constexpr scope_fail(scope_fail&& other) noexcept(std::is_nothrow_move_constructible_v<F>)
        : f(std::move(other.f)), active(other.active), exception_count(other.exception_count) {
        other.active = false;
    }

    // Deleted copy
    scope_fail(const scope_fail&)                    = delete;
    auto operator=(const scope_fail&) -> scope_fail& = delete;

    // Move assignment
    constexpr auto operator=(scope_fail&& other) noexcept(std::is_nothrow_move_assignable_v<F>) -> scope_fail& {
        if (this != &other) {
            f               = std::move(other.f);
            active          = other.active;
            exception_count = other.exception_count;
            other.active    = false;
        }
        return *this;
    }

    // Destructor: call if scope is exiting due to an exception
    ~scope_fail() noexcept(noexcept(f())) {
        if (active && std::uncaught_exceptions() > exception_count) {
            f();
        }
    }

    // Release to prevent execution
    constexpr auto release() -> void { active = false; }
};

// Factory helper
template <typename F>
constexpr auto make_scope_fail(F&& f) -> scope_fail<std::decay_t<F>> {
    return scope_fail<std::decay_t<F>>(std::forward<F>(f));
}

// TODO(CK): make a std::experimental::scope_success<EF>::scope_success conform
// implementation
template <typename F>
class [[nodiscard]] scope_success {
    F    f;
    bool active = true;
    int  exception_count{};

  public:
    // Constructor: capture current uncaught exceptions
    constexpr explicit scope_success(F func) noexcept(std::is_nothrow_move_constructible_v<F>)
        : f(std::move(func)), exception_count(std::uncaught_exceptions()) {}

    // Move constructor
    constexpr scope_success(scope_success&& other) noexcept(std::is_nothrow_move_constructible_v<F>)
        : f(std::move(other.f)), active(other.active), exception_count(other.exception_count) {
        other.active = false;
    }

    // Deleted copy
    scope_success(const scope_success&)                    = delete;
    auto operator=(const scope_success&) -> scope_success& = delete;

    // Move assignment
    constexpr auto operator=(scope_success&& other) noexcept(std::is_nothrow_move_assignable_v<F>) -> scope_success& {
        if (this != &other) {
            f               = std::move(other.f);
            active          = other.active;
            exception_count = other.exception_count;
            other.active    = false;
        }
        return *this;
    }

    // Destructor: call only if scope is exiting normally
    ~scope_success() noexcept(noexcept(f())) {
        if (active && std::uncaught_exceptions() == exception_count) {
            f();
        }
    }

    // Release to prevent execution
    constexpr auto release() -> void { active = false; }
};

// Factory helper
template <typename F>
constexpr auto make_scope_success(F&& f) -> scope_success<std::decay_t<F>> {
    return scope_success<std::decay_t<F>>(std::forward<F>(f));
}

template <typename Resource, typename Deleter>
class [[nodiscard]] unique_resource {
    Resource resource;
    Deleter  deleter;
    bool     active = true;

  public:
    // Constructor
    constexpr unique_resource(Resource r, Deleter d) noexcept(std::is_nothrow_move_constructible_v<Deleter>)
        : resource(std::move(r)), deleter(std::move(d)) {}

    // Move constructor
    constexpr unique_resource(unique_resource&& other) noexcept(std::is_nothrow_move_constructible_v<Deleter>)
        : resource(std::move(other.resource)), deleter(std::move(other.deleter)), active(other.active) {
        other.active = false;
    }

    // Move assignment
    constexpr auto operator=(unique_resource&& other) noexcept(std::is_nothrow_move_assignable_v<Deleter>)
        -> unique_resource& {
        if (this != &other) {
            reset(std::move(other.resource));
            deleter      = std::move(other.deleter);
            active       = other.active;
            other.active = false;
        }
        return *this;
    }

    // Deleted copy operations
    unique_resource(const unique_resource&)                    = delete;
    auto operator=(const unique_resource&) -> unique_resource& = delete;

    // Destructor
    ~unique_resource() noexcept(noexcept(deleter(resource))) {
        if (active) {
            deleter(resource);
        }
    }

    // Release ownership
    constexpr void release() noexcept(noexcept(deleter(resource))) { active = false; }

    // Reset resource
    constexpr void reset() noexcept(noexcept(deleter(resource))) {
        if (active) {
            deleter(resource);
        }
        active = false;
    }
    constexpr void reset(Resource new_resource) noexcept(noexcept(deleter(resource))) {
        if (active) {
            deleter(resource);
        }
        resource = std::move(new_resource);
        active   = true;
    }

    // Accessors
    constexpr auto get() const -> const Resource& { return resource; }
    constexpr auto get() -> Resource& { return resource; }

    // operator* — only for non-void pointer resources
    constexpr auto operator*() const noexcept -> std::add_lvalue_reference_t<std::remove_pointer_t<Resource>>
        requires(std::is_pointer_v<Resource> && !std::is_void_v<std::remove_pointer_t<Resource>>)
    {
        return *resource;
    }

    // operator-> — only for pointer resources
    constexpr auto operator->() const noexcept -> Resource
        requires std::is_pointer_v<Resource>
    {
        return resource;
    }

    // Check if active
    constexpr explicit operator bool() const noexcept { return active; }
};

// Deduction guide
template <typename Resource, typename Deleter>
unique_resource(Resource&&, Deleter&&) -> unique_resource<std::decay_t<Resource>, std::decay_t<Deleter>>;

} // namespace beman::scope

#endif // BEMAN_SCOPE_USE_FALLBACK

#endif // SCOPE_IMPL_HPP
