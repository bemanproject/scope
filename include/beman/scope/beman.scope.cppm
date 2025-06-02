// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// create the beman.scope.gcm in gcm.cache directory
// g++-15 -std=c++26 -O2 -fmodules -fmodule-only -c ${scopetop}/include/beman/scope/beman.scope.cppm
module;

#include "scope.hpp"

export module beman.scope;

export namespace beman::scope {
using ::beman::scope::scope_exit;
using ::beman::scope::scope_fail;
using ::beman::scope::scope_success;
using ::beman::scope::unique_resource;
} // namespace beman::scope
