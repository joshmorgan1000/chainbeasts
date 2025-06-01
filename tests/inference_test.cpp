#include "neuropet/training.hpp"
#include <gtest/gtest.h>

TEST(InferenceTest, SimpleChain) {
    neuropet::CreatureModel model;
    neuropet::Int8Layer layer{neuropet::Int8Op::Dense, 1, 1, {1}, {0}};
    model.sensor.layers.push_back(layer);
    layer.weights = {2};
    layer.bias = {1};
    model.core.layers.push_back(layer);
    layer.weights = {3};
    layer.bias = {0};
    model.appendage.layers.push_back(layer);

    std::vector<int8_t> result = neuropet::run_inference(model, {2});
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 15);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
