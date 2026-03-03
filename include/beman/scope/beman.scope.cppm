// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// create the beman.scope.gcm in gcm.cache directory
// g++-15 -std=c++26 -O2 -fmodules -fmodule-only -c ${scopetop}/include/beman/scope/beman.scope.cppm
module;

// Put all implementation-provided headers into the global module fragment
// to prevent attachment to this module.
#ifdef BEMAN_SCOPE_IMPORT_STD
import std;
#else
    #include <exception>
    #include <utility>
    #include <type_traits>

// warning: '#include <filename>' attaches the declarations to the named
// module 'beman.scope', which is not usually intended; consider moving that
// directive before the module declaration
// [-Winclude-angled-in-module-purview]
// TODO(CK): #include <filename>
#endif

#ifndef BEMAN_SCOPE_USE_DANIELA_ADVICE
    #include <beman/scope/scope_impl.hpp>
#endif

export module beman.scope;

#ifdef BEMAN_SCOPE_USE_DANIELA_ADVICE
    #define BEMAN_SCOPE_MODULE_EXPORT export
    #define BEMAN_SCOPE_MODULE_BEGIN_EXPORT export {
    #define BEMAN_SCOPE_MODULE_END_EXPORT }

    // If you define BEMAN_SCOPE_ATTACH_TO_GLOBAL_MODULE
    //  - all declarations are detached from module 'beman.scope'
    //  - the module behaves like a traditional static library, too
    //  - all library symbols are mangled traditionally
    //  - you can mix TUs with either importing or #including the {beman.scope} API
    #ifdef BEMAN_SCOPE_ATTACH_TO_GLOBAL_MODULE
extern "C++" {
    #endif

// NOTE: this export all implementation details too! CK
export {

    // FIXME: warning: '#include <filename>' attaches the declarations to the named
    // module 'beman.scope', which is not usually intended; consider moving that
    // directive before the module declaration [-Winclude-angled-in-module-purview]

    #include <beman/scope/scope_impl.hpp>

} // end of export

    #ifdef BEMAN_SCOPE_ATTACH_TO_GLOBAL_MODULE
}
    #endif

#else
// ==========================================================
// Note: not needed because of using `module attachment`!
// ==========================================================

export namespace beman::scope {
using ::beman::scope::scope_exit;
using ::beman::scope::scope_fail;
using ::beman::scope::scope_success;
using ::beman::scope::unique_resource;
} // namespace beman::scope

// ==========================================================
#endif

// TODO(CK): g++-15: sorry, unimplemented: private module fragment
// XXX module :private;

// FIXME: /usr/local/Cellar/gcc/15.2.0_1/include/c++/15/bits/refwrap.h:310:11:
// error: conflicting declaration of 'class std::reference_wrapper<_Tp>' in module 'beman.scope'
//  XXX   /usr/local/Cellar/gcc/15.2.0_1/include/c++/15/type_traits:75:11: note: previously declared in global module
