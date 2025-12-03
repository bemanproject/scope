# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

set(ALL_EXAMPLES scope_example unique_resource unique_resource-file)

# module tests will only compile with gcc15 or clang20 and above
if(CMAKE_CXX_SCAN_FOR_MODULES AND CMAKE_CXX_MODULE_STD)
    list(APPEND ALL_EXAMPLES scope-module)
endif()

message("Examples to be built: ${ALL_EXAMPLES}")

foreach(example ${ALL_EXAMPLES})
    add_executable(${example})
    target_sources(${example} PRIVATE ${example}.cpp)
    target_link_libraries(${example} PRIVATE beman::scope)
endforeach()
