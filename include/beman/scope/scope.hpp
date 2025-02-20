// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_SCOPE_HPP
#define BEMAN_SCOPE_HPP

#include <concepts>
#include <functional>
#include <exception>

namespace beman::scope {

// -- 7.6.7 Feature test macro --
//
//        __cpp_lib_scope
//

//=========================================================

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
//

//=========================================================

template <typename T>
concept HasCanInvoke = requires(T t) {
    { t.can_invoke() } -> std::convertible_to<bool>;
};

template <typename T>
concept HasStaticCanInvoke = requires {
    { T::can_invoke() } -> std::convertible_to<bool>;
};

template <typename T>
concept HasRelease = requires(T t) {
    { t.release() } -> std::same_as<void>;
};

template <typename T>
concept HasStaticRelease = requires {
    { T::release() } -> std::same_as<void>;
};

template <typename F, typename R, typename... Args>
concept invocable_return = std::invocable<F, Args...> && std::convertible_to<std::invoke_result_t<F, Args...>, R>;

template <typename F>
concept scope_exit_function =
    invocable_return<F, void> && std::is_nothrow_move_constructible_v<F> && std::is_copy_constructible_v<F>;

template <typename T>
concept scope_invoke_checker = HasStaticCanInvoke<T> || HasCanInvoke<T> || invocable_return<T, bool>;

//=========================================================

struct ExecuteAlways;

//=========================================================

template <scope_exit_function ExitFunc, scope_invoke_checker InvokeChecker = ExecuteAlways>
class scope_guard {
  public:
    explicit constexpr scope_guard(ExitFunc&& exit_func) noexcept(std::is_nothrow_constructible_v<ExitFunc>) //
        try
        : m_exit_func(std::forward<ExitFunc>(exit_func)) //
    {
    } catch (...) {
        m_exit_func();
    }

    explicit constexpr scope_guard(ExitFunc&& exit_func, InvokeChecker&& invoke_checker) noexcept(
        std::is_nothrow_constructible_v<ExitFunc> && std::is_nothrow_constructible_v<InvokeChecker>)
        : m_exit_func(std::forward<ExitFunc>(exit_func)),
          m_invoke_checker{std::forward<InvokeChecker>(invoke_checker)} //
    {}

    explicit constexpr scope_guard(scope_guard&& rhs) noexcept
        : m_exit_func{std::move(rhs)},
          m_invoke_checker{std::move(rhs.m_invoke_checker)} //
    {}

    scope_guard(const scope_guard&)            = delete;
    scope_guard& operator=(const scope_guard&) = delete;
    scope_guard& operator=(scope_guard&&)      = delete;

    constexpr ~scope_guard() noexcept(noexcept(m_exit_func())) {
        if (check_can_invoke(m_invoke_checker)) {
            m_exit_func();
        }
    }

    constexpr void release() noexcept // Shouldn't this noexcept be dependent on the noexcept of the release function?
        requires HasRelease<InvokeChecker> || HasStaticRelease<InvokeChecker>
    {
        if constexpr (HasRelease<InvokeChecker>) {
            m_invoke_checker.release();
        } else {
            InvokeChecker::release();
        }
    }

  private:
    [[no_unique_address]] ExitFunc      m_exit_func;
    [[no_unique_address]] InvokeChecker m_invoke_checker;

    template <typename T>
    static constexpr bool check_can_invoke(const T& obj) {
        if constexpr (HasStaticCanInvoke<T>) {
            return T::can_invoke();
        } else if constexpr (HasCanInvoke<T>) {
            return obj.can_invoke();
        } else {
            return std::invoke(obj);
        }
    }
};

//=========================================================

template <std::invocable ExitFunc>
scope_guard(ExitFunc&&) -> scope_guard<ExitFunc>;

template <std::invocable ExitFunc, scope_invoke_checker InvokeChecker>
scope_guard(ExitFunc&&, InvokeChecker&&) -> scope_guard<ExitFunc, InvokeChecker>;

//=========================================================
//=========================================================
//=========================================================

struct ExecuteAlways {
    [[nodiscard]] static constexpr bool can_invoke() { return true; }
};

class ExecuteWhenNoException {
  public:
    [[nodiscard]] bool can_invoke() const { return m_uncaught_on_creation >= std::uncaught_exceptions(); }

  private:
    int m_uncaught_on_creation = std::uncaught_exceptions();
};

class ExecuteOnlyWhenException {
  public:
    [[nodiscard]] bool can_invoke() const { return m_uncaught_on_creation < std::uncaught_exceptions(); }

  private:
    int m_uncaught_on_creation = std::uncaught_exceptions();
};

//=========================================================

template <typename T = void>
class Releasable;

template <>
class Releasable<void> {
  public:
    bool can_invoke() const { return m_can_invoke; }

    void release() { m_can_invoke = false; }

  private:
    bool m_can_invoke = true;
};

template <scope_invoke_checker InvokeChecker>
class Releasable<InvokeChecker> {
  public:
    Releasable() = default;

    Releasable(InvokeChecker&& invoke_checker) : InvokeChecker(std::move(invoke_checker)) {}

    bool can_invoke() const { return m_can_invoke && m_invoke_checker.can_invoke(); }

    void release() { m_can_invoke = false; }

  private:
    [[no_unique_address]] InvokeChecker m_invoke_checker = {};

    bool m_can_invoke = true;
};

//=========================================================

// -- 7.5.1 Header <scope> synopsis [scope.syn] --
//
//        namespace std {
//        template <class EF>
//        class scope_exit;
//
//        template <class EF>
//        class scope_fail;
//
//        template <class EF>
//        class scope_success;

//=========================================================

template <class ExitFunc>
using scope_exit = scope_guard<ExitFunc, Releasable<>>;

template <class ExitFunc>
using scope_success = scope_guard<ExitFunc, Releasable<ExecuteWhenNoException>>;

template <class ExitFunc>
using scope_fail = scope_guard<ExitFunc, Releasable<ExecuteOnlyWhenException>>;

//=========================================================

//
//        template <class R, class D>
//        class unique_resource;
//
//        // factory function
//        template <class R, class D, class S = decay_t<R>>
//        unique_resource<decay_t<R>, decay_t<D>>
//        make_unique_resource_checked(R&& r, const S& invalid, D&& d) noexcept(see below);
//        } // namespace std
//

//=========================================================

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

} // namespace beman::scope

#endif // BEMAN_SCOPE_HPP
