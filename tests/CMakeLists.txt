# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.8.1
    SYSTEM
    # FIXME: FIND_PACKAGE_ARGS 3.8.1
)
FetchContent_MakeAvailable(Catch2)

set(ALL_TESTNAMES scope_success scope_exit scope_fail unique_resource)

message("Tests to be built: ${ALL_TESTNAMES}")

include(Catch)

if(CMAKE_CXX_SCAN_FOR_MODULES)
    add_executable(test.module)
    target_sources(test.module PRIVATE module.test.cpp)
    target_compile_features(test.module PRIVATE cxx_std_23)
    target_link_libraries(test.module PRIVATE beman::scope)
    add_test(NAME test.module COMMAND test.module)

    # XXX NO! target_compile_options(test.module PRIVATE "-fmodules")
    # target_link_libraries(
    #     test.module
    #     PUBLIC FILE_SET TYPE CXX_MODULES include/beman/scope/beman.scope.cppm
    #     PRIVATE Catch2::Catch2WithMain beman::scope
    # )
    # catch_discover_tests(test.module)
endif()

foreach(testname ${ALL_TESTNAMES})
    add_executable(test.${testname})
    target_sources(test.${testname} PRIVATE ${testname}.test.cpp)
    target_link_libraries(
        test.${testname}
        PRIVATE Catch2::Catch2WithMain beman::scope
    )
    catch_discover_tests(test.${testname})
endforeach()
