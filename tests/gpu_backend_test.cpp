#include "neuropet/int8_kernel.hpp"
#include <gtest/gtest.h>

TEST(GpuBackendTest, FallbackWhenUnavailable) {
    EXPECT_EQ(neuropet::int8_gpu_available(), false);
    std::vector<int8_t> A{1, 2, 3, 4};
    std::vector<int8_t> B{5, 6, 7, 8};
    std::vector<int8_t> C(4);
    neuropet::int8_matmul_gpu(A, B, C, 2, 2, 2);
    EXPECT_EQ(C[0], 19);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
