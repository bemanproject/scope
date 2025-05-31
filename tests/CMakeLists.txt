# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.8.0
)
FetchContent_MakeAvailable(Catch2)

message(
    "Compiler is: ${CMAKE_CXX_COMPILER_ID} version:  ${CMAKE_CXX_COMPILER_VERSION}"
)

# module tests will only compile with gcc15 or clang20 and above
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 20)
    set(ALL_TESTNAMES scope_success scope_exit scope_fail unique_resource module)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 15)
    set(ALL_TESTNAMES scope_success scope_exit scope_fail unique_resource module)
else()
    set(ALL_TESTNAMES scope_success scope_exit scope_fail unique_resource)
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
