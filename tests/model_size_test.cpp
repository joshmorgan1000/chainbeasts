#include "neuropet/training.hpp"
#include <gtest/gtest.h>
#include <stdexcept>
#include <vector>

TEST(ModelSizeLimit, ByteCounting) {
    neuropet::CreatureModel model;
    neuropet::Int8Layer l{neuropet::Int8Op::Dense, 1, 1, {1}, {2}};
    model.core.layers.push_back(l);
    EXPECT_EQ(neuropet::model_byte_size(model), 2);
}

TEST(ModelSizeLimit, EnforceThrows) {
    neuropet::CreatureModel model;
    neuropet::Int8Layer l{neuropet::Int8Op::Dense, 1, 1};
    l.weights.resize(neuropet::DEFAULT_MAX_MODEL_BYTES + 1);
    model.core.layers.push_back(l);
    bool threw = false;
    try {
        neuropet::enforce_model_size(model);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    EXPECT_EQ(threw, true);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
