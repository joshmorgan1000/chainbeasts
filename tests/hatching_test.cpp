#include "neuropet/hatching.hpp"
#include <array>
#include <gtest/gtest.h>

TEST(HatchingTest, TopologyDerivation) {
    std::array<uint8_t, 32> seed{};
    for (size_t i = 0; i < seed.size(); ++i)
        seed[i] = static_cast<uint8_t>(i);
    auto topo = neuropet::derive_topology(seed);
    EXPECT_EQ(topo.sensor_count, 2);
    EXPECT_EQ(topo.append_count, 1);
    EXPECT_EQ(topo.sensor_size[0], 96);
    EXPECT_EQ(topo.sensor_size[1], 64);
    EXPECT_EQ(topo.sensor_size[2], 96);
    EXPECT_EQ(topo.sensor_size[3], 32);
    EXPECT_EQ(topo.append_size[0], 32);
    EXPECT_EQ(topo.append_size[1], 32);
    EXPECT_EQ(topo.append_size[2], 96);
    EXPECT_EQ(topo.append_size[3], 32);
}

TEST(HatchingTest, WeightGeneration) {
    uint64_t seed = 0;
    for (int i = 0; i < 8; ++i)
        seed = (seed << 8) | static_cast<uint64_t>(i);
    auto w = neuropet::generate_genesis_weights(5, seed);
    ASSERT_EQ(w.size(), 5u);
    EXPECT_EQ(w[0], -4);
    EXPECT_EQ(w[1], -7);
    EXPECT_EQ(w[2], 7);
    EXPECT_EQ(w[3], -4);
    EXPECT_EQ(w[4], -8);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
