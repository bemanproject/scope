// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <iostream>
#include <memory>
#include <beman/scope/scope.hpp>

constexpr size_t arr_size = 10;

int main() {
  
  {
    // Allocate an array
    auto resource_ptr = beman::scope::unique_resource
    (
       new int[arr_size], // acquire array resource
       // Cleanup function
       [](int* ptr) { delete[] ptr;
                      std::cout << "Array deleted.\n"; }
    );

    // Use the array
    for (size_t i = 0; i < arr_size; ++i) {
        resource_ptr.get()[i] = static_cast<int>(i * 2);
    }
    std::cout << "First element: " << resource_ptr.get()[0] << "\n";
  }

  // Resource is automatically released when `resource_ptr` goes out of scope
  std::cout << "After scope: \n";

}
