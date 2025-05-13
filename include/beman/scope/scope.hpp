// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_SCOPE_HPP
#define BEMAN_SCOPE_HPP

#include <type_traits>
#include <utility>

#if __cplusplus < 202002L
  #error "C++20 or later is required"
#endif

#include <experimental/scope>

namespace beman::scope {

// -- 7.6.7 Feature test macro --
//
//        __cpp_lib_scope
//

// -- 7.5.1 Header <scope> synopsis [scope.syn] --
//
//        namespace std {

//        template <class EF>
//        class scope_exit;
template <class EF>
using scope_exit = std::experimental::scope_exit<EF>;

//        template <class EF>
//        class scope_fail;
template <class EF>
using scope_fail = std::experimental::scope_fail<EF>;

//        template <class EF>
//        class scope_success;
template <class EF>
using scope_success = std::experimental::scope_success<EF>;

//        template <class R, class D>
//        class unique_resource;
template <class R, class D>
using unique_resource = std::experimental::unique_resource<R, D>;

//        // factory function
//        template <class R, class D, class S = decay_t<R>>
//        unique_resource<decay_t<R>, decay_t<D>>
//        make_unique_resource_checked(R&& r, const S& invalid, D&& d) noexcept(see below);

template <class R, class D, class S = std::decay_t<R>>
unique_resource<std::decay_t<R>, std::decay_t<D>>
make_unique_resource_checked(R&& r, const S& invalid, D&& d) noexcept(noexcept(
    std::experimental::make_unique_resource_checked(std::forward(r), std::forward(invalid), std::forward(d)))) {
    return std::experimental::make_unique_resource_checked(std::forward(r), std::forward(invalid), std::forward(d));
}

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
//
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
