# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.8.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(tests.scope)
target_sources(tests.scope PRIVATE scope.test.cpp)

target_link_libraries(tests.scope PRIVATE Catch2::Catch2WithMain)

target_include_directories(
    tests.scope
    PRIVATE ${CMAKE_SOURCE_DIR}/include
)

include(CTest)
include(Catch)
catch_discover_tests(tests.scope)
