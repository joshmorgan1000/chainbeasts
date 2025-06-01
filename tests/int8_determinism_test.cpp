#include "neuropet/int8_kernel.hpp"
#include <gtest/gtest.h>

TEST(Int8KernelDeterminism, RepeatedExecutionStable) {
    std::vector<int8_t> A{1, 2, 3, 4};
    std::vector<int8_t> B{5, 6, 7, 8};
    std::vector<int8_t> first(4);
    std::vector<int8_t> next(4);

    neuropet::int8_matmul(A, B, first, 2, 2, 2);
    for (int i = 0; i < 5; ++i) {
        neuropet::int8_matmul(A, B, next, 2, 2, 2);
        EXPECT_EQ(next, first);
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
