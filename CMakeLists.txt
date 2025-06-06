# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

if(CMAKE_CXX_STANDARD GREATER_EQUAL 23)
    if(CMAKE_CXX_MODULE_STD)
        if(CMAKE_VERSION VERSION_EQUAL 4.0.2)
            set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
                "a9e1cf81-9932-4810-974b-6eccaf14e457"
            )
        else()
            set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
                "d0edc3af-4c50-42ea-a356-e2862fe7a444"
            )
        endif()
        set(CMAKE_CXX_SCAN_FOR_MODULES 1)
        add_definitions(-DHAS_STDLIB_MODULES)
    endif()
    set(CMAKE_CXX_STANDARD_REQUIRED OFF)
endif()

cmake_minimum_required(VERSION 4.0)

project(
    beman.scope
    DESCRIPTION "Generic Scope Guard"
    LANGUAGES CXX
    VERSION 0.0.1
)

# gersemi: off

# Modules opt in only on compilers that support g++-15 and clang-19+
if(CMAKE_CXX_STANDARD GREATER_EQUAL 23)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19)
        if(APPLE AND CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 20.1.5)
            execute_process(OUTPUT_VARIABLE LLVM_PREFIX COMMAND brew --prefix llvm@20 COMMAND_ECHO STDOUT)
            string(STRIP ${LLVM_PREFIX} LLVM_PREFIX)
            file(REAL_PATH ${LLVM_PREFIX} LLVM_ROOT)
            set(LLVM_ROOT ${LLVM_ROOT} CACHE PATH "")
            message(STATUS "LLVM_ROOT=${LLVM_ROOT}")

            add_compile_options(-fexperimental-library)
            add_link_options(-L${LLVM_ROOT}/lib/c++ -lc++experimental)
            add_compile_options(-stdlib=libc++)
            add_link_options(-stdlib=libc++)
        endif()
        set(CMAKE_CXX_SCAN_FOR_MODULES 1)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 15)
        set(CMAKE_CXX_SCAN_FOR_MODULES 1)
    endif()
else()
    set(CMAKE_CXX_SCAN_FOR_MODULES 0)
endif()

if(CMAKE_CXX_SCAN_FOR_MODULES)
    set(CMAKE_CXX_STANDARD_REQUIRED OFF)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)
if(CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
    find_program(CLANG_TIDY_EXE NAMES clang-tidy clang-tidy-20 DOC "Path to clang-tidy executable")
endif()

include(CMakePrintHelpers)
cmake_print_variables(
    CMAKE_CXX_STANDARD
    CMAKE_CXX_SCAN_FOR_MODULES
    CMAKE_CXX_MODULE_STD
    CMAKE_CXX_COMPILER_IMPORT_STD
)

# [CMAKE.SKIP_TESTS]
option(
    BEMAN_SCOPE_BUILD_TESTS
    "Enable building tests and test infrastructure. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

# [CMAKE.SKIP_EXAMPLES]
option(
    BEMAN_SCOPE_BUILD_EXAMPLES
    "Enable building examples. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

option(
    BEMAN_SCOPE_INSTALL_CONFIG_FILE_PACKAGE
    "Enable creating and installing a CMake config-file package. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

message(STATUS "Compiler is: ${CMAKE_CXX_COMPILER_ID} version: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "CMake is: ${CMAKE_VERSION} modules scan: ${CMAKE_CXX_SCAN_FOR_MODULES}")

if(CMAKE_CXX_SCAN_FOR_MODULES)
    add_library(beman.scope)
    target_sources(
        beman.scope
        PUBLIC
            FILE_SET HEADERS
            BASE_DIRS include
            FILES include/beman/scope/scope.hpp
        PUBLIC
            FILE_SET CXX_MODULES
            BASE_DIRS include
            FILES include/beman/scope/beman.scope.cppm
    )
    # prevent "not include "cxx_std_20" (or newer) among its `target_compile_features`; no C++ standard found"
    target_compile_features(beman.scope PUBLIC cxx_std_23)
else()
    add_library(beman.scope INTERFACE)
    target_sources(
        beman.scope
        INTERFACE
            FILE_SET HEADERS
            BASE_DIRS include
            FILES include/beman/scope/scope.hpp
    )
    target_compile_features(beman.scope INTERFACE cxx_std_20)
endif()

add_library(beman::scope ALIAS beman.scope)

set_target_properties(
    beman.scope
    PROPERTIES
        VERIFY_INTERFACE_HEADER_SETS ON
        EXPORT_NAME scope
)

include(GNUInstallDirs)

install(
    TARGETS beman.scope
    COMPONENT beman.scope
    EXPORT beman.scope-targets

    FILE_SET CXX_MODULES
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILE_SET HEADERS
)

# gersemi: on

if(BEMAN_SCOPE_INSTALL_CONFIG_FILE_PACKAGE)
    include(CMakePackageConfigHelpers)

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/beman.scope-config-version.cmake
        COMPATIBILITY ExactVersion
    )

    install(
        FILES
            cmake/beman.scope-config.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/beman.scope-config-version.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/beman.scope
        COMPONENT beman.scope
    )

    install(
        EXPORT beman.scope-targets
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/beman.scope
        NAMESPACE beman::
        CXX_MODULES_DIRECTORY
        cxx-modules
        COMPONENT beman.scope
    )
endif()

if(BEMAN_SCOPE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(BEMAN_SCOPE_BUILD_EXAMPLES)
    if(CLANG_TIDY_EXE)
        message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
        set(CMAKE_CXX_CLANG_TIDY
            "${CLANG_TIDY_EXE};-extra-arg=--Wno-error=unknown-argument"
        )
    endif()
    add_subdirectory(examples)
endif()
