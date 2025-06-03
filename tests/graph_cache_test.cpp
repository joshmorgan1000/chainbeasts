#include <gtest/gtest.h>
#include <harmonics/cycle.hpp>
#include <harmonics/graph.hpp>
#include <harmonics/parser.hpp>

TEST(GraphCacheTest, CompilesOnlyOncePerDigest) {
    const char* src1 = "producer p; consumer c; cycle { p -> c; }";
    const char* src2 = "producer p; consumer c; layer l; cycle { p -> l; l -> c; }";

    harmonics::Parser p1{src1};
    auto g1 = harmonics::build_graph(p1.parse_declarations());
    harmonics::Parser p2{src2};
    auto g2 = harmonics::build_graph(p2.parse_declarations());

    harmonics::compile_cycle_kernel_compiles() = 0;
    harmonics::compile_fpga_cycle_kernel_compiles() = 0;

    harmonics::AutoPrecisionPolicy policy{};
    (void)harmonics::compile_cycle_kernels(g1, policy);
    EXPECT_EQ(harmonics::compile_cycle_kernel_compiles(), 1);
    (void)harmonics::compile_cycle_kernels(g1, policy);
    EXPECT_EQ(harmonics::compile_cycle_kernel_compiles(), 1);

    (void)harmonics::compile_cycle_kernels(g2, policy);
    EXPECT_EQ(harmonics::compile_cycle_kernel_compiles(), 2);

    (void)harmonics::compile_fpga_cycle_kernels(g1);
    EXPECT_EQ(harmonics::compile_fpga_cycle_kernel_compiles(), 1);
    (void)harmonics::compile_fpga_cycle_kernels(g1);
    EXPECT_EQ(harmonics::compile_fpga_cycle_kernel_compiles(), 1);

    (void)harmonics::compile_fpga_cycle_kernels(g2);
    EXPECT_EQ(harmonics::compile_fpga_cycle_kernel_compiles(), 2);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
