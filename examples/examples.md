# Scope Examples

The header-based examples include `<beman/scope.hpp>` or an individual component header. The module example imports `beman.scope` and `std` and requires a compiler and build configuration that support those modules.

| Example | Description |
| --- | --- |
| [`scope_example.cpp`](scope_example.cpp) | Compares manual scope handling with `scope_exit`, `scope_fail`, and `scope_success`. |
| [`unique_resource.cpp`](unique_resource.cpp) | Manages a dynamically allocated integer array with `unique_resource` automatically releases it with `delete[]` on scope exit. |
| [`unique_resource_file.cpp`](unique_resource_file.cpp) | Manages a `FILE*` opened with `fopen`, checks for an open failure and automatically closes the file with `fclose` on scope exit. |
| [`unique_resource_fd.cpp`](unique_resource_fd.cpp) | Owns a POSIX integer file descriptor directly, checks `-1` for open failure, and closes the descriptor on normal or early return. Built on Unix platforms; pass a file path as its argument. |
| [`scope_exit_stream_flags.cpp`](scope_exit_stream_flags.cpp) | Uses `scope_exit` to restore a stream's formatting flags after temporary hexadecimal formatting. Uses only standard C++ facilities. |
| [`scope_module.cpp`](scope_module.cpp) | Uses `import beman.scope` and `import std` to demonstrate scope guard and `unique_resource`. |
