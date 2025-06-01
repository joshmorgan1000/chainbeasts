#include "neuropet/training.hpp"
#include <gtest/gtest.h>

TEST(SensorTest, SensorNetworkProcessesInput) {
    neuropet::CreatureModel model;
    neuropet::Int8Layer dense{neuropet::Int8Op::Dense, 1, 1, {2}, {0}};
    neuropet::Int8Layer relu{neuropet::Int8Op::ReLU, 1, 1, {}, {}};
    model.sensor.layers.push_back(dense);
    model.sensor.layers.push_back(relu);
    dense.weights = {3};
    model.core.layers.push_back(dense);
    model.core.layers.push_back(relu);
    dense.weights = {4};
    model.appendage.layers.push_back(dense);

    auto out = neuropet::run_inference(model, {1});
    ASSERT_EQ(out.size(), 1);
    EXPECT_EQ(out[0], 24);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
