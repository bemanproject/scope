# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.8.0
)
FetchContent_MakeAvailable(Catch2)

set(ALL_TESTNAMES scope unique_resource)

message("Tests to be built: ${ALL_TESTS}")

include(CTest)
include(Catch)


foreach(testname ${ALL_TESTNAMES})
    add_executable(test.${testname})
    target_sources(test.${testname} PRIVATE ${testname}.test.cpp)
    target_include_directories(test.${testname} PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(test.${testname} PRIVATE Catch2::Catch2WithMain)
endforeach()


catch_discover_tests(test.scope test.unique_resource)
