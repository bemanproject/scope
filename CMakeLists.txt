# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.25)

project(
    beman.scope
    DESCRIPTION "Generic Scope Guard"
    LANGUAGES CXX
    VERSION 0.0.1
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

add_library(beman.scope INTERFACE)
add_library(beman::scope ALIAS beman.scope)

# gersemi: off
set_target_properties(
    beman.scope
    PROPERTIES
        VERIFY_INTERFACE_HEADER_SETS ON
        EXPORT_NAME scope
)

target_sources(
    beman.scope
    INTERFACE
        FILE_SET HEADERS
        BASE_DIRS include
        FILES include/beman/scope/scope.hpp
)

install(
    TARGETS beman.scope
    EXPORT beman.scope-targets
    COMPONENT beman.scope
    FILE_SET HEADERS
)
# gersemi: on

if(BEMAN_SCOPE_INSTALL_CONFIG_FILE_PACKAGE)
    include(GNUInstallDirs)
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
        COMPONENT beman.scope
    )
endif()

if(BEMAN_SCOPE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(BEMAN_SCOPE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
