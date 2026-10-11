// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_SCOPE_UNIQUE_RESOURCE_HPP
#define BEMAN_SCOPE_UNIQUE_RESOURCE_HPP

#include <functional>
#include <type_traits>
#include <utility>

#if !defined(__cpp_concepts) || __cpp_concepts < 201907L
#error "C++20 or later is required"
#endif

namespace beman::scope {

/// Owns a resource and invokes its deleter on reset or destruction unless released.
/// Deleter calls must not throw. Resource and deleter construction may throw.
template <class R, class D>
class [[nodiscard]] unique_resource {
    static_assert(std::is_object_v<R> || std::is_lvalue_reference_v<R>);
    static_assert(std::is_object_v<D>);
    static_assert(std::is_nothrow_move_constructible_v<R> || std::is_copy_constructible_v<R>);
    static_assert(std::is_nothrow_move_constructible_v<D> || std::is_copy_constructible_v<D>);

    using storage_type =
        std::conditional_t<std::is_lvalue_reference_v<R>, std::reference_wrapper<std::remove_reference_t<R>>, R>;

    template <class T, class U>
    static constexpr bool safely_constructible =
        std::is_constructible_v<T, U> && (std::is_nothrow_constructible_v<T, U> || std::is_constructible_v<T, U&>);

    template <class T, class U>
    static constexpr bool nothrow_construction =
        std::is_nothrow_constructible_v<T, U> || std::is_nothrow_constructible_v<T, U&>;

    struct ownership_tag {};

    // Returning prvalues constructs directly into the member. Rollback runs before
    // any successfully initialized member is destroyed, including during constexpr evaluation.
    template <class T, class U, class Cleanup>
    static constexpr T construct(U&& value, Cleanup cleanup) noexcept(nothrow_construction<T, U>) {
        if constexpr (std::is_nothrow_constructible_v<T, U>) {
            return T(std::forward<U>(value));
        } else if constexpr (std::is_nothrow_constructible_v<T, U&>) {
            return T(value);
        } else {
            try {
                return T(value);
            } catch (...) {
                cleanup();
                throw;
            }
        }
    }

    constexpr decltype(auto) resource_value() noexcept {
        if constexpr (std::is_lvalue_reference_v<R>) {
            return resource.get();
        } else {
            return (resource);
        }
    }

    constexpr D transfer_deleter(unique_resource& rhs) {
        if constexpr (std::is_nothrow_move_constructible_v<D>) {
            return D(std::move(rhs.deleter));
        } else {
            try {
                return D(rhs.deleter);
            } catch (...) {
                // A nonthrowing resource move has already transferred the handle.
                // If it was copied instead, rhs still owns it and needs no rollback.
                if constexpr (std::is_nothrow_move_constructible_v<storage_type>) {
                    if (rhs.execute_on_reset) {
                        rhs.release();
                        rhs.deleter(resource_value());
                    }
                }
                throw;
            }
        }
    }

    template <class RR, class DD>
    constexpr unique_resource(RR&& r, DD&& d, bool active, ownership_tag) noexcept(
        nothrow_construction<storage_type, RR> && nothrow_construction<D, DD>)
        : resource(construct<storage_type>(std::forward<RR>(r),
                                           [&] {
                                               if (active) {
                                                   d(r);
                                               }
                                           })),
          deleter(construct<D>(std::forward<DD>(d),
                               [&] {
                                   if (active) {
                                       d(resource_value());
                                   }
                               })),
          execute_on_reset(active) {}

    template <class RR, class DD, class S>
    friend constexpr unique_resource<std::decay_t<RR>, std::decay_t<DD>> make_unique_resource_checked(
        RR&& r, const S& invalid, DD&& d) noexcept(std::is_nothrow_constructible_v<std::decay_t<RR>, RR> &&
                                                   std::is_nothrow_constructible_v<std::decay_t<DD>, DD>);

  public:
    /// Value-initializes the resource and deleter without taking ownership.
    constexpr unique_resource() noexcept(std::is_nothrow_default_constructible_v<storage_type> &&
                                         std::is_nothrow_default_constructible_v<D>)
        requires(std::is_default_constructible_v<R> && std::is_default_constructible_v<D>)
        : resource(), deleter(), execute_on_reset(false) {}

    /// Takes ownership, cleaning up the incoming resource if member construction fails.
    template <class RR, class DD>
        requires(safely_constructible<storage_type, RR> && safely_constructible<D, DD>)
    constexpr unique_resource(RR&& r,
                              DD&& d) noexcept(nothrow_construction<storage_type, RR> && nothrow_construction<D, DD>)
        : unique_resource(std::forward<RR>(r), std::forward<DD>(d), true, ownership_tag{}) {}

    /// Transfers ownership only after both members have been initialized.
    constexpr unique_resource(unique_resource&& rhs) noexcept(std::is_nothrow_move_constructible_v<storage_type> &&
                                                              std::is_nothrow_move_constructible_v<D>)
        : resource(std::move_if_noexcept(rhs.resource)),
          deleter(transfer_deleter(rhs)),
          execute_on_reset(std::exchange(rhs.execute_on_reset, false)) {}

    unique_resource(const unique_resource&)            = delete;
    unique_resource& operator=(const unique_resource&) = delete;

    /// Cleans up the destination before transferring the source resource and deleter.
    constexpr unique_resource&
    operator=(unique_resource&& rhs) noexcept(std::is_nothrow_move_assignable_v<storage_type> &&
                                              std::is_nothrow_move_assignable_v<D>)
        requires((std::is_nothrow_move_assignable_v<storage_type> || std::is_copy_assignable_v<storage_type>) &&
                 (std::is_nothrow_move_assignable_v<D> || std::is_copy_assignable_v<D>))
    {
        if (this == &rhs) {
            return *this;
        }
        reset();
        // Perform potentially throwing copies before moves that change the source.
        if constexpr (std::is_nothrow_move_assignable_v<storage_type>) {
            if constexpr (std::is_nothrow_move_assignable_v<D>) {
                resource = std::move(rhs.resource);
                deleter  = std::move(rhs.deleter);
            } else {
                deleter  = rhs.deleter;
                resource = std::move(rhs.resource);
            }
        } else {
            resource = rhs.resource;
            if constexpr (std::is_nothrow_move_assignable_v<D>) {
                deleter = std::move(rhs.deleter);
            } else {
                deleter = rhs.deleter;
            }
        }
        execute_on_reset = std::exchange(rhs.execute_on_reset, false);
        return *this;
    }

    constexpr ~unique_resource() { reset(); }

    /// Cleans up once and leaves the wrapper inactive.
    constexpr void reset() noexcept {
        if (std::exchange(execute_on_reset, false)) {
            deleter(resource_value());
        }
    }

    /// Replaces the resource. On assignment failure, cleans up the incoming resource.
    template <class RR>
        requires(std::is_nothrow_assignable_v<storage_type&, RR> ||
                 std::is_assignable_v<storage_type&, const std::remove_reference_t<RR>&>)
    constexpr void reset(RR&& r) {
        reset();
        if constexpr (std::is_nothrow_assignable_v<storage_type&, RR>) {
            resource = std::forward<RR>(r);
        } else {
            try {
                resource = std::as_const(r);
            } catch (...) {
                deleter(r);
                throw;
            }
        }
        execute_on_reset = true;
    }

    /// Disables cleanup without changing the stored resource.
    constexpr void release() noexcept { execute_on_reset = false; }

    constexpr const R& get() const noexcept {
        if constexpr (std::is_lvalue_reference_v<R>) {
            return resource.get();
        } else {
            return resource;
        }
    }

    constexpr const D& get_deleter() const noexcept { return deleter; }

    constexpr std::add_lvalue_reference_t<std::remove_pointer_t<R>> operator*() const noexcept
        requires(std::is_pointer_v<R> && !std::is_void_v<std::remove_pointer_t<R>>)
    {
        return *get();
    }

    constexpr R operator->() const noexcept
        requires std::is_pointer_v<R>
    {
        return get();
    }

  private:
    // Ordinary members preserve guaranteed copy elision from construct(). A
    // potentially overlapping subobject would permit another, throwing move.
    storage_type resource;
    D            deleter;
    bool         execute_on_reset;
};

template <class R, class D>
unique_resource(R, D) -> unique_resource<R, D>;

/// Takes ownership only when r differs from invalid. Comparison must not throw.
template <class R, class D, class S = std::decay_t<R>>
constexpr unique_resource<std::decay_t<R>, std::decay_t<D>> make_unique_resource_checked(
    R&& r, const S& invalid, D&& d) noexcept(std::is_nothrow_constructible_v<std::decay_t<R>, R> &&
                                             std::is_nothrow_constructible_v<std::decay_t<D>, D>) {
    using result_type = unique_resource<std::decay_t<R>, std::decay_t<D>>;
    const bool active = !bool(r == invalid);
    return result_type(std::forward<R>(r), std::forward<D>(d), active, typename result_type::ownership_tag{});
}

} // namespace beman::scope

#endif // BEMAN_SCOPE_UNIQUE_RESOURCE_HPP
