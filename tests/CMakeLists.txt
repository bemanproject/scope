# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.8.0
)
FetchContent_MakeAvailable(Catch2)

set(ALL_TESTNAMES scope_success scope_exit scope_fail unique_resource)

message("Tests to be built: ${ALL_TESTNAMES}")

include(CTest)
include(Catch)

foreach(testname ${ALL_TESTNAMES})
    add_executable(test.${testname})
    target_sources(test.${testname} PRIVATE ${testname}.test.cpp)
    target_include_directories(
        test.${testname}
        PRIVATE ${CMAKE_SOURCE_DIR}/include
    )
    target_link_libraries(test.${testname} PRIVATE Catch2::Catch2WithMain)
    catch_discover_tests(test.${testname})
endforeach()
