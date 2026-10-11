// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_SCOPE_SCOPE_GUARD_HPP
#define BEMAN_SCOPE_SCOPE_GUARD_HPP

#include <concepts>
#include <exception>
#include <limits>
#include <type_traits>
#include <utility>

#if !defined(__cpp_concepts) || __cpp_concepts < 201907L
#error "C++20 or later is required"
#endif

namespace beman::scope {

// Template argument `ScopeExitFunc` shall be
//   - a function object type([function.objects]),
//   - lvalue reference to function,
//   - or lvalue reference to function object type.
//
//   If `ScopeExitFunc` is an object type, it shall meet the requirements of Cpp17Destructible(Table 30).
//   Given an lvalue g of type remove_reference_t<EF>, the expression g() shall be well- formed.

//==================================================================================================

// --- Concepts ---

/// Checks whether invoking `F` with `Args...` produces a value convertible to `R`.
template <typename F, typename R, typename... Args>
concept invocable_return = std::invocable<F, Args...> && std::convertible_to<std::invoke_result_t<F, Args...>, R>;

/// Describes a callable that can be stored and invoked by a scope guard.
template <typename F>
concept scope_exit_function =
    invocable_return<F, void> && (std::is_nothrow_move_constructible_v<F> || std::is_copy_constructible_v<F>);

/// Checks whether a callable can decide whether a scope guard should invoke its exit function.
template <typename T>
concept scope_function_invoke_check = invocable_return<T, bool>;

/// Checks whether `T` provides an instance `release()` member function.
template <typename T>
concept HasRelease = requires(T t) {
    { t.release() } -> std::same_as<void>;
};

/// Checks whether `T` provides a static `release()` member function.
template <typename T>
concept HasStaticRelease = requires {
    { T::release() } -> std::same_as<void>;
};

// --- Enum ---

/// Controls whether the exit function runs if a scope guard constructor throws.
enum class exception_during_construction_behaviour {
    /// Do not invoke the exit function when construction fails.
    dont_invoke_exit_func,
    /// Invoke the exit function when construction fails.
    invoke_exit_func
};

//==================================================================================================

/// Generalized scope guard that conditionally invokes a callable at scope exit.
template <scope_exit_function ScopeExitFunc,
          typename InvokeChecker = void,
          exception_during_construction_behaviour ConstructionExceptionBehavior =
              exception_during_construction_behaviour::invoke_exit_func>
class [[nodiscard]] scope_guard;

//==================================================================================================

/** Generalized scope guard template
 * @tparam ScopeExitFunc callable invoked when the guard is active at scope exit.
 * @tparam InvokeChecker callable that decides whether `ScopeExitFunc` is invoked.
 * @tparam ConstructionExceptionBehavior behavior when construction of the guard fails.
 */
template <scope_exit_function                     ScopeExitFunc,
          scope_function_invoke_check             InvokeChecker,
          exception_during_construction_behaviour ConstructionExceptionBehavior>
class [[nodiscard]] scope_guard<ScopeExitFunc, InvokeChecker, ConstructionExceptionBehavior> {
  public:
    /** The constructor parameter `exit_func` in the following constructors shall be :
     *  - a reference to a function
     *  - or a reference to a function object([function.objects])
     *
     *
     * If EFP is not an lvalue reference type and is_nothrow_constructible_v<EF,EFP> is true,
     * initialize exit_function with std::forward<EFP>(f);
     * otherwise initialize exit_function with f.
     *
     * scope_fail / scope_exit
     * If the initialization of exit_function throws an exception, calls f().
     *
     * scope_success
     *  [Note: If initialization of exit_function fails, f() won't be called. end note]
     */
    /// Constructs a guard from an exit function and an invocation checker.
    template <typename EF, typename CHKR>
    constexpr scope_guard(EF&&   exit_func,
                          CHKR&& invoke_checker) noexcept(std::is_nothrow_constructible_v<ScopeExitFunc> &&
                                                          std::is_nothrow_constructible_v<InvokeChecker>) try
        : exit_func{std::forward<EF>(exit_func)}, invoke_check_func{std::forward<CHKR>(invoke_checker)} {
    } catch (...) {
        if constexpr (ConstructionExceptionBehavior == exception_during_construction_behaviour::invoke_exit_func) {
            exit_func();

            // To throw? or not to throw?
            throw;
        }
    }

    /// Constructs a guard from an exit function and a default-constructed checker.
    template <typename EF>
    explicit constexpr scope_guard(EF&& exit_func) noexcept(std::is_nothrow_constructible_v<ScopeExitFunc> &&
                                                            std::is_nothrow_constructible_v<InvokeChecker>)
        requires(std::is_default_constructible_v<InvokeChecker> && !std::is_same_v<std::remove_cvref<EF>, scope_guard>)
    try : exit_func{std::forward<EF>(exit_func)} {
    } catch (...) {
        if constexpr (ConstructionExceptionBehavior == exception_during_construction_behaviour::invoke_exit_func) {
            exit_func();

            // To throw? or not to throw?
            throw;
        }
    }

    /// Moves a guard and releases the moved-from guard's invocation checker.
    constexpr scope_guard(scope_guard&& rhs) noexcept(std::is_nothrow_move_constructible_v<ScopeExitFunc> &&
                                                      std::is_nothrow_move_constructible_v<InvokeChecker>)
        requires(HasRelease<InvokeChecker> || HasStaticRelease<InvokeChecker>)
        : exit_func{std::move(rhs.exit_func)}, invoke_check_func{std::move(rhs.invoke_check_func)} {
        // TODO: This does not work correctly for a shared invoke checker
        //       After a move will disable all.

        if constexpr (HasStaticRelease<InvokeChecker>) {
            InvokeChecker::release();
        } else {
            rhs.release();
        }
    }

    scope_guard(const scope_guard&)            = delete;
    scope_guard& operator=(const scope_guard&) = delete;
    scope_guard& operator=(scope_guard&& rhs)  = delete;

    /// Invokes the exit function when the checker says the guard is still active.
    constexpr ~scope_guard() noexcept(noexcept(exit_func()) && noexcept(invoke_check_func())) {
        if (invoke_check_func()) {
            exit_func();
        }
    }

    /// Returns the invocation checker stored by this guard.
    InvokeChecker& invoke_checker() & noexcept { return invoke_checker; }

    /// Disables invocation of the exit function for this guard.
    constexpr void release() noexcept
        // Shouldn't this "noexcept" be dependent on the noexcept of the release function? how??
        requires(HasRelease<InvokeChecker> || HasStaticRelease<InvokeChecker>)
    {
        if constexpr (HasRelease<InvokeChecker>) {
            invoke_check_func.release();
        } else {
            InvokeChecker::release();
        }
    }

  private:
    ScopeExitFunc exit_func;
    InvokeChecker invoke_check_func;
};

//======

// --- Specializations for no releaser

/// Scope guard specialization that invokes its exit function when construction fails.
template <scope_exit_function ScopeExitFunc>
class [[nodiscard]] scope_guard<ScopeExitFunc, void, exception_during_construction_behaviour::invoke_exit_func> {
    ScopeExitFunc exit_func;

  public:
    /// Constructs a guard from an exit function.
    template <typename T>
    explicit constexpr scope_guard(T&& exit_func) noexcept(std::is_nothrow_constructible_v<ScopeExitFunc>)
        requires(!std::is_same_v<std::remove_cvref<T>, scope_guard>)
    try : exit_func(std::forward<T>(exit_func)) {
    } catch (...) {
        exit_func();

        throw;
    }

    scope_guard(const scope_guard&)            = delete;
    scope_guard(scope_guard&&)                 = delete;
    scope_guard& operator=(const scope_guard&) = delete;
    scope_guard& operator=(scope_guard&&)      = delete;

    /// Invokes the exit function on destruction.
    constexpr ~scope_guard() noexcept(noexcept(exit_func())) { exit_func(); }
};

//======

/// Scope guard specialization that does not invoke its exit function when construction fails.
template <scope_exit_function ScopeExitFunc>
class [[nodiscard]] scope_guard<ScopeExitFunc, void, exception_during_construction_behaviour::dont_invoke_exit_func> {
    ScopeExitFunc exit_func;

  public:
    /// Constructs a guard from an exit function.
    template <typename T>
    explicit constexpr scope_guard(T&& exit_func) noexcept(std::is_nothrow_constructible_v<ScopeExitFunc>)
        requires(!std::is_same_v<std::remove_cvref<T>, scope_guard>)
        : exit_func(std::forward<T>(exit_func)) {}

    scope_guard(const scope_guard&)            = delete;
    scope_guard(scope_guard&&)                 = delete;
    scope_guard& operator=(const scope_guard&) = delete;
    scope_guard& operator=(scope_guard&&)      = delete;

    /// Invokes the exit function on destruction.
    constexpr ~scope_guard() noexcept(noexcept(exit_func())) { exit_func(); }
};

//==================================================================================================

// --- Deduction guides ---

/// Deduction guide for a guard with an explicit invocation checker.
template <typename ExitFunc,
          typename InvokeChecker,
          exception_during_construction_behaviour ecdb = exception_during_construction_behaviour::invoke_exit_func>
    requires(scope_exit_function<ExitFunc> && (scope_function_invoke_check<InvokeChecker>))
scope_guard(ExitFunc&&, InvokeChecker&&) -> scope_guard<std::decay_t<ExitFunc>, std::decay_t<InvokeChecker>, ecdb>;

/// Deduction guide for a guard with a default invocation checker.
template <typename ExitFunc,
          typename InvokeChecker                       = void,
          exception_during_construction_behaviour ecdb = exception_during_construction_behaviour::invoke_exit_func>
    requires(scope_exit_function<ExitFunc> &&
             (scope_function_invoke_check<InvokeChecker> || std::is_void_v<InvokeChecker>))
scope_guard(ExitFunc&&) -> scope_guard<std::decay_t<ExitFunc>, InvokeChecker, ecdb>;

//==================================================================================================

/// Invocation checker used by `scope_exit`.
class releaser {
  public:
    /// Returns whether the exit function is still enabled.
    bool operator()() const { return can_invoke; }

    /// Disables the exit function.
    void release() { can_invoke = false; }

  private:
    bool can_invoke = true;
};

//======

/// Invocation checker that enables execution only when no exception is active.
class releaseable_execute_when_no_exception {
  public:
    /// Marker used to select the construction behavior for `scope_success`.
    using DontInvokeOnCreationException = void;

    /// Returns whether destruction is occurring without a new exception.
    [[nodiscard]] bool operator()() const noexcept(noexcept(std::uncaught_exceptions())) {
        return uncaught_on_creation >= std::uncaught_exceptions();
    }

    /// Disables the exit function.
    void release() { uncaught_on_creation = std::numeric_limits<int>::min(); }

  private:
    int uncaught_on_creation = std::uncaught_exceptions();
};

//======

/// Invocation checker that enables execution only while unwinding an exception.
class releaseable_execute_only_when_exception {
  public:
    /// Returns whether destruction is occurring during exception unwinding.
    [[nodiscard]] bool operator()() const noexcept(noexcept(std::uncaught_exceptions())) {
        return uncaught_on_creation < std::uncaught_exceptions();
    }

    /// Disables the exit function.
    void release() { uncaught_on_creation = std::numeric_limits<int>::max(); }

  private:
    int uncaught_on_creation = std::uncaught_exceptions();
};

//==================================================================================================

// --- type aliases ---

/// Executes a callable on every scope exit unless released.
template <class ExitFunc>
using scope_exit = scope_guard<ExitFunc, releaser, exception_during_construction_behaviour::invoke_exit_func>;

/// Executes a callable only when the scope exits without an exception.
template <class ExitFunc>
using scope_success = scope_guard<ExitFunc,
                                  releaseable_execute_when_no_exception,
                                  exception_during_construction_behaviour::dont_invoke_exit_func>;

/// Executes a callable only when the scope exits during exception unwinding.
template <class ExitFunc>
using scope_fail = scope_guard<ExitFunc,
                               releaseable_execute_only_when_exception,
                               exception_during_construction_behaviour::invoke_exit_func>;

} // namespace beman::scope

#endif // BEMAN_SCOPE_SCOPE_GUARD_HPP
