# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

set(ALL_EXAMPLES scope_example unique_resource)

message("Examples to be built: ${ALL_EXAMPLES}")

foreach(example ${ALL_EXAMPLES})
    add_executable(${example})
    target_sources(${example} PRIVATE ${example}.cpp)
    target_include_directories(${example} PRIVATE ${CMAKE_SOURCE_DIR}/include)
endforeach()
