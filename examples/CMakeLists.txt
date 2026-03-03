# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.30...4.3)

include(../cmake/prelude.cmake)

project(beman.scope.example LANGUAGES CXX)

if(PROJECT_IS_TOP_LEVEL)
    include(../cmake/cxx-modules-rules.cmake)

    find_package(beman.scope 0.1.0 EXACT REQUIRED)

    option(BEMAN_SCOPE_IMPORT_STD "Use import std;" ${BEMAN_HAS_IMPORT_STD})

    enable_testing()
endif()

set(ALL_EXAMPLES scope_example unique_resource unique_resource-file)

message("Examples to be built: ${ALL_EXAMPLES}")

foreach(example ${ALL_EXAMPLES})
    add_executable(${example})
    target_sources(${example} PRIVATE ${example}.cpp)
    target_link_libraries(${example} PRIVATE beman::scope_headers)
    add_test(NAME ${example} COMMAND ${example})
endforeach()

# module tests will only compile with gcc15 or clang20 and above
# NOTE: needs C++23 or newer! CK
if(BEMAN_USE_MODULES)
    add_executable(scope_module scope-module.cpp)
    target_link_libraries(scope_module PRIVATE beman::scope)
    if(BEMAN_SCOPE_IMPORT_STD)
        set_target_properties(scope_module PROPERTIES CXX_MODULE_STD ON)
    endif()
    add_test(NAME scope_module COMMAND scope_module)
endif()
