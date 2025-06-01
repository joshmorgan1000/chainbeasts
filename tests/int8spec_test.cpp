#include "neuropet/int8_spec.hpp"
#include <cstdio>
#include <gtest/gtest.h>

TEST(Int8SpecTest, SaveLoadRoundtrip) {
    neuropet::Int8Network net;
    neuropet::Int8Layer l{neuropet::Int8Op::Dense, 1, 1, {2}, {1}};
    net.layers.push_back(l);
    const char* path = "test_net.bin";
    neuropet::save_network(net, path);
    auto loaded = neuropet::load_network(path);
    std::remove(path);
    ASSERT_EQ(loaded.layers.size(), net.layers.size());
    EXPECT_EQ(loaded.layers[0].op, net.layers[0].op);
    EXPECT_EQ(loaded.layers[0].weights[0], 2);
    EXPECT_EQ(loaded.layers[0].bias[0], 1);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
