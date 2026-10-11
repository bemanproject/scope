// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/scope/unique_resource.hpp>

#include <cstdio>
#include <fcntl.h>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>

// POSIX file descriptors are integers: zero is valid and -1 means failure.
// unique_resource stores the int directly; unique_ptr would need a custom
// nullable handle type to represent these rules.
int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file>\n";
        return 1;
    }

    auto file = beman::scope::make_unique_resource_checked(
        ::open(argv[1], O_RDONLY), -1, [](int fd) noexcept { ::close(fd); });

    if (file.get() == -1) {
        std::perror("open");
        return 1; // The checked factory prevents calling close(-1).
    }

    struct stat info {};
    if (::fstat(file.get(), &info) == -1) {
        std::perror("fstat");
        return 1; // The descriptor is closed even on this early return.
    }

    std::cout << "File size: " << info.st_size << " bytes\n";
    // The descriptor is also closed on normal return or stack unwinding.
}
