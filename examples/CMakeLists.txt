# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.28...4.2)

project(beman.scope.example LANGUAGES CXX)

if(PROJECT_IS_TOP_LEVEL)
    if(NOT DEFINED CMAKE_CXX_STANDARD)
        set(CMAKE_CXX_STANDARD 23)
        set(CMAKE_CXX_EXTENSIONS YES)
        set(CMAKE_CXX_STANDARD_REQUIRED YES)
    endif()
    set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

    find_package(beman.scope REQUIRED)

    enable_testing()
endif()

set(ALL_EXAMPLES scope_example unique_resource unique_resource-file)

# module tests will only compile with gcc15 or clang20 and above
# NOTE: needs C++23 or newer! CK
if(CMAKE_CXX_SCAN_FOR_MODULES)
    list(APPEND ALL_EXAMPLES scope-module)
endif()

message("Examples to be built: ${ALL_EXAMPLES}")

foreach(example ${ALL_EXAMPLES})
    add_executable(${example})
    target_sources(${example} PRIVATE ${example}.cpp)
    target_link_libraries(${example} PRIVATE beman::scope)
    add_test(NAME ${example} COMMAND ${example})
endforeach()
