# Scope Examples

The header-based examples include `<beman/scope.hpp>`. The module example imports `beman.scope` and `std` and requires a compiler and build configuration that support those modules.

| Example | Description |
| --- | --- |
| [`scope_example.cpp`](scope_example.cpp) | Compares manual scope handling with `scope_exit`, `scope_fail`, and `scope_success`. |
| [`unique_resource.cpp`](unique_resource.cpp) | Manages a dynamically allocated integer array with `unique_resource` automatically releases it with `delete[]` on scope exit. |
| [`unique_resource_file.cpp`](unique_resource_file.cpp) | Manages a `FILE*` opened with `fopen`, checks for an open failure and automatically closes the file with `fclose` on scope exit. |
| [`scope_module.cpp`](scope_module.cpp) | Uses `import beman.scope` and `import std` to demonstrate scope guard and `unique_resource`. |
