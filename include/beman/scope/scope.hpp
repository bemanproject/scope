// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_SCOPE_HPP
#define BEMAN_SCOPE_HPP

#include <concepts>
#include <exception>
#include <type_traits>
#include <version>

#if (!(defined(__cpp_concepts) || !(defined(__cpp_lib_concepts))))
static_assert(false, "C++20 Concepts Required");
#endif

#ifdef BEMAN_SCOPE_USE_STD_EXPERIMENTAL

#include <experimental/scope>

namespace beman::scope {

template <class EF>
using scope_exit = std::experimental::scope_exit<EF>;

template <class EF>
using scope_fail = std::experimental::scope_fail<EF>;

template <class EF>
using scope_success = std::experimental::scope_success<EF>;

template <class R, class D>
using unique_resource = std::experimental::unique_resource<R, D>;

template <class R, class D, class S = std::decay_t<R>>
unique_resource<std::decay_t<R>, std::decay_t<D>>
make_unique_resource_checked(R&& r, const S& invalid, D&& d) noexcept(noexcept(
    std::experimental::make_unique_resource_checked(std::forward(r), std::forward(invalid), std::forward(d)))) {
    return std::experimental::make_unique_resource_checked(std::forward(r), std::forward(invalid), std::forward(d));
}

} // namespace beman::scope

#else // ! BEMAN_SCOPE__USE_STD_EXPERIMENTAL

namespace beman::scope {

//==================================================================================================
//
// -- 7.6.7 Feature test macro --
//
//        __cpp_lib_scope
//

// -- 7.5.1 Header <scope> synopsis [scope.syn] --
//
//        namespace std {
//
//        template <class EF>
//        class scope_exit;
//
//        template <class EF>
//        class scope_fail;
//
//        template <class EF>
//        class scope_success;
//
//        template <class R, class D>
//        class unique_resource;
//
//        // factory function
//        template <class R, class D, class S = decay_t<R>>
//        unique_resource<decay_t<R>, decay_t<D>>
//        make_unique_resource_checked(R&& r, const S& invalid, D&& d) noexcept(see below);
//
//        } // namespace std
//

// -- 7.5.2 Scope guard class templates [scope.scope_guard] --
//
//        template <class EF>
//        class scope_guard {
//        public:
//            template <class EFP>
//            explicit scope_guard(EFP&& f) noexcept(see below);
//
//            scope_guard(scope_guard&& rhs) noexcept(see below);
//
//            scope_guard(const scope_guard&)            = delete;
//            scope_guard& operator=(const scope_guard&) = delete;
//            scope_guard& operator=(scope_guard&&)      = delete;
//
//            ~scope_guard() noexcept(see below);
//
//            void release() noexcept;
//
//        private:
//            EF   exit_function;                               // exposition only
//            bool execute_on_destruction{true};                // exposition only
//            int  uncaught_on_creation{uncaught_exceptions()}; // exposition only
//        };
//
//        template <class EF>
//        scope_guard(EF) -> scope_guard<EF>;

// -- 7.6.1 Class template unique_resource [scope.unique_resource.class] --
//
//        template <class R, class D>
//        class unique_resource {
//        public:
//            unique_resource();
//            template <class RR, class DD>
//            unique_resource(RR&& r, DD&& d) noexcept(see below);
//            unique_resource(unique_resource&& rhs) noexcept(see below);
//            ~unique_resource();
//            unique_resource& operator=(unique_resource&& rhs) noexcept(see below);
//            void             reset() noexcept;
//            template <class RR>
//            void      reset(RR&& r);
//            void      release() noexcept;
//            const R&  get() const noexcept;
//            see below operator*() const noexcept;
//            R         operator->() const noexcept;
//            const D&  get_deleter() const noexcept;
//
//        private:
//            using R1 = conditional_t<is_reference_v<R>,                        //
//                                    reference_wrapper<remove_reference_t<R>>, //
//                                    R>;                                       // exposition only
//
//            R1   resource;               // exposition only
//            D    deleter;                // exposition only
//            bool execute_on_reset{true}; // exposition only
//        };
//
//        template <typename R, typename D>
//        unique_resource(R, D) -> unique_resource<R, D>;
//
//==================================================================================================

// Template argument `ScopeExitFunc` shall be
//   - a function object type([function.objects]),
//   - lvalue reference to function,
//   - or lvalue reference to function object type.
//
//   If `ScopeExitFunc` is an object type, it shall meet the requirements of Cpp17Destructible(Table 30).
//   Given an lvalue g of type remove_reference_t<EF>, the expression g() shall be well- formed.

//==================================================================================================

// --- Concepts ---
template <typename F, typename R, typename... Args>
concept invocable_return = std::invocable<F, Args...> && std::convertible_to<std::invoke_result_t<F, Args...>, R>;

template <typename F>
concept scope_exit_function =
    invocable_return<F, void> && (std::is_nothrow_move_constructible_v<F> || std::is_copy_constructible_v<F>);

template <typename T>
concept scope_function_invoke_check = invocable_return<T, bool>;

template <typename T>
concept HasRelease = requires(T t) {
    { t.release() } -> std::same_as<void>;
};

template <typename T>
concept HasStaticRelease = requires {
    { T::release() } -> std::same_as<void>;
};

// --- Enum ---

enum class exception_during_constuction_behaviour : std::uint8_t {
    dont_invoke_exit_func,
    invoke_exit_func,
};

//==================================================================================================

//  --- `scope_guard` - primary template ---

template <scope_exit_function ScopeExitFunc,
          typename InvokeChecker = void,
          exception_during_constuction_behaviour ConstructionExceptionBehavior =
              exception_during_constuction_behaviour::invoke_exit_func>
class [[nodiscard]] scope_guard;

//==================================================================================================

// --- General definition ---

template <scope_exit_function                    ScopeExitFunc,
          scope_function_invoke_check            InvokeChecker,
          exception_during_constuction_behaviour ConstructionExceptionBehavior>
class [[nodiscard]] scope_guard<ScopeExitFunc, InvokeChecker, ConstructionExceptionBehavior> {
  public:
    // The constructor parameter `exit_func` in the following constructors shall be :
    //  - a reference to a function
    //  - or a reference to a function object([function.objects])
    //

    // If EFP is not an lvalue reference type and is_nothrow_constructible_v<EF,EFP> is true,
    // initialize exit_function with std::forward<EFP>(f);
    // otherwise initialize exit_function with f.

    // scope_fail / scope_exit
    // If the initialization of exit_function throws an exception, calls f().

    // scope_success
    //  [Note: If initialization of exit_function fails, f() won’t be called. —end note]

    template <typename EF, typename CHKR>
    constexpr scope_guard(EF&&   exit_func,
                          CHKR&& invoke_checker) noexcept(std::is_nothrow_constructible_v<ScopeExitFunc> &&
                                                          std::is_nothrow_constructible_v<InvokeChecker>) try
        : m_exit_func{std::forward<EF>(exit_func)}, m_invoke_checker{std::forward<CHKR>(invoke_checker)} {
    } catch (...) {
        if constexpr (ConstructionExceptionBehavior == exception_during_constuction_behaviour::invoke_exit_func) {
            exit_func();

            // To throw? or not to throw?
            throw;
        }
    }

    template <typename EF>
    explicit constexpr scope_guard(EF&& exit_func) noexcept(std::is_nothrow_constructible_v<ScopeExitFunc> &&
                                                            std::is_nothrow_constructible_v<InvokeChecker>)
        requires(std::is_default_constructible_v<InvokeChecker> && !std::is_same_v<std::remove_cvref<EF>, scope_guard>)
    try : m_exit_func{std::forward<EF>(exit_func)} {
    } catch (...) {
        if constexpr (ConstructionExceptionBehavior == exception_during_constuction_behaviour::invoke_exit_func) {
            exit_func();

            // To throw? or not to throw?
            throw;
        }
    }

    constexpr scope_guard(scope_guard&& rhs) noexcept(std::is_nothrow_move_constructible_v<ScopeExitFunc> &&
                                                      std::is_nothrow_move_constructible_v<InvokeChecker>)
        requires(HasRelease<InvokeChecker> || HasStaticRelease<InvokeChecker>)
        : m_exit_func{std::move(rhs.m_exit_func)}, m_invoke_checker{std::move(rhs.m_invoke_checker)} {
        // TODO: This does not work corectly for a shared invoke checker
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

    constexpr ~scope_guard() noexcept(noexcept(m_exit_func()) && noexcept(m_invoke_checker())) {
        if (m_invoke_checker()) {
            m_exit_func();
        }
    }

    InvokeChecker& invoke_checker() & noexcept { return m_invoke_checker; }

    constexpr void release() noexcept
        // Shouldn't this "noexcept" be dependent on the noexcept of the release function? how??
        requires(HasRelease<InvokeChecker> || HasStaticRelease<InvokeChecker>)
    {
        if constexpr (HasRelease<InvokeChecker>) {
            m_invoke_checker.release();
        } else {
            InvokeChecker::release();
        }
    }

  private:
    ScopeExitFunc m_exit_func;
    InvokeChecker m_invoke_checker;
};

//======

// --- Specializations for no releaser

template <scope_exit_function ScopeExitFunc>
class [[nodiscard]] scope_guard<ScopeExitFunc, void, exception_during_constuction_behaviour::invoke_exit_func> {
    ScopeExitFunc m_exit_func;

  public:
    template <typename T>
    explicit constexpr scope_guard(T&& exit_func) noexcept(std::is_nothrow_constructible_v<ScopeExitFunc>)
        requires(!std::is_same_v<std::remove_cvref<T>, scope_guard>)
    try : m_exit_func(std::forward<T>(exit_func)) {
    } catch (...) {
        exit_func();

        throw;
    }

    scope_guard(const scope_guard&)            = delete;
    scope_guard(scope_guard&&)                 = delete;
    scope_guard& operator=(const scope_guard&) = delete;
    scope_guard& operator=(scope_guard&&)      = delete;

    constexpr ~scope_guard() noexcept(noexcept(m_exit_func())) { m_exit_func(); }
};

//======

template <scope_exit_function ScopeExitFunc>
class [[nodiscard]] scope_guard<ScopeExitFunc, void, exception_during_constuction_behaviour::dont_invoke_exit_func> {
    ScopeExitFunc m_exit_func;

  public:
    template <typename T>
    explicit constexpr scope_guard(T&& exit_func) noexcept(std::is_nothrow_constructible_v<ScopeExitFunc>)
        requires(!std::is_same_v<std::remove_cvref<T>, scope_guard>)
        : m_exit_func(std::forward<T>(exit_func)) {}

    scope_guard(const scope_guard&)            = delete;
    scope_guard(scope_guard&&)                 = delete;
    scope_guard& operator=(const scope_guard&) = delete;
    scope_guard& operator=(scope_guard&&)      = delete;

    constexpr ~scope_guard() noexcept(noexcept(m_exit_func())) { m_exit_func(); }
};

//==================================================================================================

// --- Deduction guides ---

template <typename ExitFunc,
          typename InvokeChecker,
          exception_during_constuction_behaviour ecdb = exception_during_constuction_behaviour::invoke_exit_func>
    requires(scope_exit_function<ExitFunc> && (scope_function_invoke_check<InvokeChecker>))
scope_guard(ExitFunc&&, InvokeChecker&&) -> scope_guard<std::decay_t<ExitFunc>, std::decay_t<InvokeChecker>, ecdb>;

template <typename ExitFunc,
          typename InvokeChecker                      = void,
          exception_during_constuction_behaviour ecdb = exception_during_constuction_behaviour::invoke_exit_func>
    requires(scope_exit_function<ExitFunc> &&
             (scope_function_invoke_check<InvokeChecker> || std::is_void_v<InvokeChecker>))
scope_guard(ExitFunc&&) -> scope_guard<std::decay_t<ExitFunc>, InvokeChecker, ecdb>;

//==================================================================================================

class Releaser {
  public:
    bool operator()() const { return m_can_invoke; }

    void release() { m_can_invoke = false; }

  private:
    bool m_can_invoke = true;
};

//======

class ReleasableExecuteWhenNoException {
  public:
    using DontInvokeOnCreationException = void;

    [[nodiscard]] bool operator()() const noexcept(noexcept(std::uncaught_exceptions())) {
        return m_uncaught_on_creation >= std::uncaught_exceptions();
    }

    void release() { m_uncaught_on_creation = INT_MIN; }

  private:
    int m_uncaught_on_creation = std::uncaught_exceptions();
};

//======

class ReleasableExecuteOnlyWhenException {
  public:
    [[nodiscard]] bool operator()() const noexcept(noexcept(std::uncaught_exceptions())) {
        return m_uncaught_on_creation < std::uncaught_exceptions();
    }

    void release() { m_uncaught_on_creation = INT_MAX; }

  private:
    int m_uncaught_on_creation = std::uncaught_exceptions();
};

//==================================================================================================

// --- type aliases ---

template <class ExitFunc>
using scope_exit = scope_guard<ExitFunc, Releaser, exception_during_constuction_behaviour::invoke_exit_func>;

template <class ExitFunc>
using scope_success = scope_guard<ExitFunc,
                                  ReleasableExecuteWhenNoException,
                                  exception_during_constuction_behaviour::dont_invoke_exit_func>;

template <class ExitFunc>
using scope_fail = scope_guard<ExitFunc,
                               ReleasableExecuteOnlyWhenException,
                               exception_during_constuction_behaviour::invoke_exit_func>;

} // namespace beman::scope

#endif // BEMAN_SCOPE__USE_STD_EXPERIMENTAL

#endif // BEMAN_SCOPE_HPP
