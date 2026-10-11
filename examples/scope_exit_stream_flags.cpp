// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/scope/scope_guard.hpp>

#include <iomanip>
#include <iostream>

void print_hex(std::ostream& output, unsigned value) {
    // Restore the stream's previous flags on scope exit, including on early
    // return or an exception. Capture the saved flags by value.
    auto restore_flags = beman::scope::scope_exit(
        [&output, flags = output.flags()]() noexcept { output.flags(flags); });

    output << std::hex << std::showbase << value << '\n';
}

int main() {
    std::cout << "Before: " << 42 << '\n';
    print_hex(std::cout, 42);
    std::cout << "After: " << 42 << '\n';
}
