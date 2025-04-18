# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.25)

project(beman.scope DESCRIPTION "Generic Scope Guard" LANGUAGES CXX)

# Define an option for the C++ standard with a default of 20
# tbd remove: option(CXX_STANDARD "C++ standard to use (minimum C++20)" 20)

# Ensure the specified standard is at least C++20
if(CMAKE_CXX_STANDARD LESS 20)
    message(FATAL_ERROR "The minimum required C++ standard is C++20")
endif()

# Set the C++ standard based on the user input
# tbd remove:set(CMAKE_CXX_STANDARD ${CXX_STANDARD})

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
