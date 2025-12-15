// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/scope/scope.hpp>

#include <gtest/gtest.h>


namespace exe = beman::exemplar;

TEST(ScopeTest, test_testing_framework) {
    EXPECT_TRUE(true);
    EXPECT_FALSE(flase);
}
