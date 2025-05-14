# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.25)

project(beman.scope DESCRIPTION "Generic Scope Guard" LANGUAGES CXX)

enable_testing()

# [CMAKE.SKIP_TESTS]
option(
    BEMAN_SCOPE_BUILD_TESTS
    "Enable building tests and test infrastructure. Default: ON. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

# [CMAKE.SKIP_EXAMPLES]
option(
    BEMAN_SCOPE_BUILD_EXAMPLES
    "Enable building examples. Default: ON. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

include(GNUInstallDirs)

# todo rm add_subdirectory(src/beman/scope)

if(BEMAN_SCOPE_BUILD_TESTS)
    add_subdirectory(tests)
endif()

if(BEMAN_SCOPE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
