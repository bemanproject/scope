# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.30...4.3)

include(./cmake/prelude.cmake)

project(
    beman.scope
    DESCRIPTION "Generic Scope Guard"
    LANGUAGES CXX
    VERSION 0.1.0
)

# Modules opt in only on compilers that support it: msvc, g++-15 and clang-20+
include(./cmake/cxx-modules-rules.cmake)

#===============================================================================
if(BEMAN_USE_MODULES)
    option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

    set(CMAKE_CXX_SCAN_FOR_MODULES ON)
    set(CMAKE_CXX_VISIBILITY_PRESET hidden)
    set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
endif()
#===============================================================================

# gersemi: off

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
message(STATUS "Cmake is: ${CMAKE_VERSION} modules scan: ${CMAKE_CXX_SCAN_FOR_MODULES}")

# gersemi: on

set(BEMAN_SCOPE_TARGETS beman.scope_headers)

if(BEMAN_USE_MODULES)
    option(BEMAN_SCOPE_IMPORT_STD "Use import std;" ${BEMAN_HAS_IMPORT_STD})

    add_library(beman.scope)
    include(GenerateExportHeader)
    generate_export_header(
        beman.scope
        BASE_NAME beman.scope
        EXPORT_FILE_NAME beman/scope/modules_export.hpp
    )

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # ---------------------------------------------------------------------------
        # TODO(daniela): I do mean C++20
        # ---------------------------------------------------------------------------
        target_compile_options(
            beman.scope
            PUBLIC
                -fsized-deallocation
                -faligned-allocation
                -Wno-include-angled-in-module-purview
        )
    endif()

    if(BEMAN_SCOPE_IMPORT_STD)
        # ---------------------------------------------------------------------------
        # Ensure binaries find the correct libstdc++ at runtime when using a
        # non-system GCC (e.g. /opt/gcc-16). Without this, CTest fails because the
        # dynamic linker finds the system's older libstdc++ which lacks required
        # GLIBCXX symbols.
        # ---------------------------------------------------------------------------
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            set(_suffix .so)
            if(APPLE)
                set(_suffix .dylib)
            endif()
            execute_process(
                COMMAND
                    ${CMAKE_CXX_COMPILER} -print-file-name=libstdc++${_suffix}
                OUTPUT_VARIABLE _gcc_stdcxx_path
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
            if(_gcc_stdcxx_path)
                cmake_path(GET _gcc_stdcxx_path PARENT_PATH _gcc_lib_dir)
                get_filename_component(_gcc_lib_dir "${_gcc_lib_dir}" REALPATH)
                list(APPEND CMAKE_BUILD_RPATH "${_gcc_lib_dir}")
                message(STATUS "GCC libstdc++ rpath: ${_gcc_lib_dir}")
            endif()
        endif()
        set_target_properties(beman.scope PROPERTIES CXX_MODULE_STD ON)
        target_compile_definitions(beman.scope PUBLIC BEMAN_SCOPE_IMPORT_STD)
    endif()
    if(BEMAN_SCOPE_USE_DANIELA_ADVICE)
        target_compile_definitions(
            beman.scope
            PUBLIC BEMAN_SCOPE_USE_DANIELA_ADVICE
        )
    endif()
    add_library(beman::scope ALIAS beman.scope)

    target_sources(
        beman.scope
        PUBLIC
            FILE_SET HEADERS
                BASE_DIRS include ${CMAKE_CURRENT_BINARY_DIR}
                FILES
                    include/beman/scope/scope.hpp
                    include/beman/scope/scope_impl.hpp
                    ${CMAKE_CURRENT_BINARY_DIR}/beman/scope/modules_export.hpp
        PUBLIC
            FILE_SET CXX_MODULES
                BASE_DIRS include
                FILES include/beman/scope/beman.scope.cppm
    )

    # ---------------------------------------------------------------------------
    # CMake requires the language standard to be specified as compile feature
    # when a target provides C++23 modules and the target will be installed
    # ---------------------------------------------------------------------------
    target_compile_features(beman.scope PUBLIC cxx_std_${CMAKE_CXX_STANDARD})

    # ---------------------------------------------------------------------------
    # FIXME: Quickfix only to prevent linker problems on windows with dll
    #        it should not needed and it does not help? CK
    # We have NO exported symbols and so NO generated .lib on windows!
    # ---------------------------------------------------------------------------
    if(WIN32 AND BUILD_SHARED_LIBS)
        set_target_properties(
            beman.scope
            PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON
        )
    endif()

    set_target_properties(beman.scope PROPERTIES EXPORT_NAME scope)
    list(APPEND BEMAN_SCOPE_TARGETS beman.scope)
endif()

add_library(beman.scope_headers INTERFACE)
target_sources(
    beman.scope_headers
    INTERFACE
        FILE_SET HEADERS
            BASE_DIRS include ${CMAKE_CURRENT_BINARY_DIR}
            FILES
                include/beman/scope/scope.hpp
                include/beman/scope/scope_impl.hpp
)

add_library(beman::scope_headers ALIAS beman.scope_headers)

set_target_properties(
    beman.scope_headers
    PROPERTIES
        VERIFY_INTERFACE_HEADER_SETS ${PROJECT_IS_TOP_LEVEL}
        EXPORT_NAME scope_headers
)

include(GNUInstallDirs)

set(package_install_dir ${CMAKE_INSTALL_LIBDIR}/cmake/beman.scope)

# ---------------------------------------------------------------------------
# TBD: install targets always? CK
# ---------------------------------------------------------------------------
install(
    TARGETS ${BEMAN_SCOPE_TARGETS}
    COMPONENT beman.scope
    EXPORT beman.scope-targets
    FILE_SET CXX_MODULES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    CXX_MODULES_BMI
        DESTINATION
            ${package_install_dir}/bmi-${CMAKE_CXX_COMPILER_ID}_$<CONFIG>
    FILE_SET HEADERS
)

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
        DESTINATION ${package_install_dir}
        COMPONENT beman.scope
    )

    install(
        EXPORT beman.scope-targets
        DESTINATION ${package_install_dir}
        NAMESPACE beman::
        CXX_MODULES_DIRECTORY
        cxx-modules
        COMPONENT beman.scope
    )
endif()

# ---------------------------------------------------------------------------
# NOTE: we use ctest in examples too! CK
# ---------------------------------------------------------------------------
enable_testing()

if(BEMAN_SCOPE_BUILD_TESTS)
    add_subdirectory(tests)
endif()

if(BEMAN_SCOPE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
