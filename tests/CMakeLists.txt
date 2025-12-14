# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.11.0
    EXCLUDE_FROM_ALL
    # FIND_PACKAGE_ARGS 3.11
)
FetchContent_MakeAvailable(Catch2)

set(ALL_TESTNAMES scope_success scope_exit scope_fail unique_resource)

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
