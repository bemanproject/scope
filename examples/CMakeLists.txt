# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

set(ALL_EXAMPLES scope_example)

message("Examples to be built: ${ALL_EXAMPLES}")

foreach(example ${ALL_EXAMPLES})
    add_executable(beman.scope.examples.${example})
    target_sources(beman.scope.examples.${example} PRIVATE ${example}.cpp)
    target_link_libraries(beman.scope.examples.${example} beman::scope)
endforeach()
