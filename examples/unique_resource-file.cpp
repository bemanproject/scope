// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <iostream>
#include <memory>
#include <cstdio>
#include <beman/scope/scope.hpp>

// clang-format off
int main() {

  {
    auto file = beman::scope::unique_resource(
        fopen("example.txt", "w"), // Acquire the FILE*
        [](FILE* f) {
            if (f) {
                std::cout << "Closing file.\n";
                fclose(f); // Release (cleanup) the resource
            }
        }
    );

    if (!file.get()) {
        std::cerr << "Failed to open file.\n";
        return 1;
    }
  }

  // Resource is automatically released when `file` goes out of scope
  std::cout << "File has been closed \n";

}
