# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

include(FetchContent)

if(APPLE AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(_FIND_PACKAGE_ARGS)
else()
    set(_FIND_PACKAGE_ARGS FIND_PACKAGE_ARGS 3.13)
endif()

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.13.0
    EXCLUDE_FROM_ALL
    ${_FIND_PACKAGE_ARGS}
)
FetchContent_MakeAvailable(Catch2)

set(ALL_TESTNAMES
    scope_success
    scope_exit
    scope_fail
    unique_resource
    unique_resource_2
)

# module tests will only compile with gcc15 or clang20 and above
if(CMAKE_CXX_SCAN_FOR_MODULES)
    list(APPEND ALL_TESTNAMES module)
endif()

message("Tests to be built: ${ALL_TESTNAMES}")

include(Catch)

foreach(testname ${ALL_TESTNAMES})
    add_executable(test.${testname})
    target_sources(test.${testname} PRIVATE ${testname}.test.cpp)
    target_link_libraries(
        test.${testname}
        PRIVATE Catch2::Catch2WithMain beman::scope
    )
    catch_discover_tests(test.${testname})
endforeach()

if(BEMAN_SCOPE_INSTALL_CONFIG_FILE_PACKAGE)
    # test if the targets are usable from the install directory
    add_test(
        NAME install-to-stagedir
        COMMAND
            ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR} --prefix
            ${CMAKE_BINARY_DIR}/stagedir --config $<CONFIG>
    )
    add_test(
        NAME find-package-test
        COMMAND
            ${CMAKE_CTEST_COMMAND} # --verbose
            --output-on-failure -C $<CONFIG> --build-and-test
            "${CMAKE_SOURCE_DIR}/examples"
            "${CMAKE_CURRENT_BINARY_DIR}/find-package-test" --build-generator
            ${CMAKE_GENERATOR} --build-makeprogram ${CMAKE_MAKE_PROGRAM}
            --build-options #
            "-D BEMAN_SCOPE_IMPORT_STD=${BEMAN_SCOPE_IMPORT_STD}"
            "-D CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}"
            "-D CMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}"
            "-D CMAKE_CXX_EXTENSIONS=${CMAKE_CXX_EXTENSIONS}"
            "-D CMAKE_CXX_SCAN_FOR_MODULES=${CMAKE_CXX_SCAN_FOR_MODULES}"
            "-D CMAKE_BUILD_TYPE=$<CONFIG>"
            "-D CMAKE_PREFIX_PATH=${CMAKE_BINARY_DIR}/stagedir"
    )
endif()
